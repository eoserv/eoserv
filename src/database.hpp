#ifndef DATABASE_HPP_INCLUDED
#define DATABASE_HPP_INCLUDED

#include <list>
#include <map>
#include <string>
#include <exception>

#ifdef DATABASE_MYSQL
#include "socket.hpp"
#include <mysql.h>
#endif // DATABASE_MYSQL
#ifdef DATABASE_SQLITE
#include <sqlite3.h>
#endif // DATABASE_SQLITE

#ifndef DATABASE_MYSQL
#ifndef DATABASE_SQLITE
#error At least one database driver must be selected
#endif // DATABASE_SQLITE
#endif // DATABASE_MYSQL

class Database_Result;
class Database;

#include "util.hpp"

/**
 * Generic Database exception type
 */
class Database_Exception : public std::exception
{
	protected:
		const char *err;
	public:
		Database_Exception(const char *e) : err(e) {};
		const char *error() { return err; };
};

/**
 * Exception thrown when an invalid Database Engine was specified
 */
class Database_InvalidEngine : public Database_Exception { public: Database_InvalidEngine(const char *e) : Database_Exception(e){} };

/**
 * Exception thrown when opening a Database failed
 */
class Database_OpenFailed : public Database_Exception { public: Database_OpenFailed(const char *e) : Database_Exception(e){} };

/**
 * Exception thrown when a Database Query failed
 */
class Database_QueryFailed : public Database_Exception { public: Database_QueryFailed(const char *e) : Database_Exception(e){} };

/**
 * Result from a Database Query containing the SELECTed rows, and/or affected row counts and error information
 */
class Database_Result : public std::list<std::map<std::string, util::variant> >
{
	protected:
		int affected_rows;
		bool error;

	public:
		/**
		 * Returns the number of affected rows from an UPDATE or INSERT query
		 */
		int AffectedRows();

		/**
		 * Returns true when an error has occured
		 */
		bool Error();

	friend class Database;
};

/**
 * Maintains and interfaces with a connection to a database
 */
class Database
{
	public:
		enum Engine
		{
			MySQL,
			SQLite
		};

	protected:
		union
		{
#ifdef DATABASE_MYSQL
			MYSQL *mysql_handle;
#endif // DATABASE_MYSQL
#ifdef DATABASE_SQLITE
			sqlite3 *sqlite_handle;
#endif // DATABASE_SQLITE
		};

		bool connected;
		Engine engine;

	public:
		/**
		 * Constructs a zombie Database object that should have Connect() called on it before anything else
		 */
		Database();

		/**
		 * Opens a connection to a database
		 * @throw Databse_InvalidEngine
		 * @throw Databse_OpenFailed
		 */
		Database(Database::Engine type, std::string host, std::string user, std::string pass, std::string db);

		/**
		 * Opens a connection to a database
		 * @throw Databse_InvalidEngine
		 * @throw Databse_OpenFailed
		 */
		void Connect(Database::Engine type, std::string host, std::string user, std::string pass, std::string db);

		/**
		 * Executes a query and returns it's result
		 * @throw Databse_QueryFailed
		 */
		Database_Result Query(const char *format, ...);

		/**
		 * Object used to collect information from an external callback
		 */
		Database_Result callbackdata;
};

#endif // DATABASE_HPP_INCLUDED
