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

class Database_Exception : public std::exception
{
	protected:
		const char *err;
	public:
		Database_Exception(const char *e) : err(e) {};
		const char *error() { return err; };
};
class Database_InvalidEngine : public Database_Exception { public: Database_InvalidEngine(const char *e) : Database_Exception(e){} };
class Database_OpenFailed : public Database_Exception { public: Database_OpenFailed(const char *e) : Database_Exception(e){} };
class Database_QueryFailed : public Database_Exception { public: Database_QueryFailed(const char *e) : Database_Exception(e){} };

class Database_Result : public std::list<std::map<std::string, util::variant> >
{
	protected:
		int affected_rows;
		bool error;

	public:
		int AffectedRows();
		bool Error();

	friend class Database;
};

class Database
{
	public:
		enum Engine
		{
			MySQL,
			SQLite
		};

	private:

	protected:
		union handle
		{
#ifdef DATABASE_MYSQL
			MYSQL *mysql_handle;
#endif // DATABASE_MYSQL
#ifdef DATABASE_SQLITE
			sqlite3 *sqlite_handle;
#endif // DATABASE_SQLITE
		} handle;
		bool connected;
		Engine engine;

	public:
		Database();
		Database(Database::Engine type, std::string host, std::string user, std::string pass, std::string db);

		void Connect(Database::Engine type, std::string host, std::string user, std::string pass, std::string db);

		Database_Result Query(const char *format, ...);

		Database_Result callbackdata;
};

#endif // DATABASE_HPP_INCLUDED
