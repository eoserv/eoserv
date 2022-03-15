/* database.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef DATABASE_HPP_INCLUDED
#define DATABASE_HPP_INCLUDED

#include "util/variant.hpp"

#include <algorithm>
#include <exception>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "platform.h"

/**
 * Generic Database exception type
 */
class Database_Exception : public std::exception
{
	protected:
		const char *err;
	public:
		Database_Exception(const char *e) : err(e) {};
		const char *error() const noexcept { return err; };
		virtual const char *what() const noexcept { return "Database_Exception"; }
};

/**
 * Exception thrown when opening a Database failed
 */
class Database_OpenFailed : public Database_Exception
{
	public: Database_OpenFailed(const char *e) : Database_Exception(e) {}
	const char *what() const noexcept { return "Database_OpenFailed"; }
};

/**
 * Exception thrown when a Database Query failed
 */
class Database_QueryFailed : public Database_Exception
{
	public: Database_QueryFailed(const char *e) : Database_Exception(e) {}
	const char *what() const noexcept { return "Database_QueryFailed"; }
};

/**
 * Result from a Database Query containing the SELECTed rows, and/or affected row counts and error information
 */
class Database_Result : public std::vector<std::unordered_map<std::string, util::variant>>
{
	protected:
		int affected_rows;
		bool error;

	public:
		Database_Result()
			: affected_rows(0)
			, error(false)
		{ }

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
		struct impl_;

		std::unique_ptr<impl_> impl;

		bool connected;
		Engine engine;

		std::string host, user, pass, db;
		unsigned int port;

		bool in_transaction;
		std::list<std::string> transaction_log;

	public:
		struct Bulk_Query_Context
		{
			Database& db;
			bool pending;

			Bulk_Query_Context(Database&);
			Bulk_Query_Context(const Bulk_Query_Context&) = delete;
			bool Pending() const;
			void RawQuery(const std::string& query);
			void Commit();
			void Rollback();
			~Bulk_Query_Context();
		};

		/**
		 * Constructs a zombie Database object that should have Connect() called on it before anything else
		 */
		Database();

		/**
		 * Opens a connection to a database if connectnow is true
		 * @param connectnow Whether to connect now or on a query request
		 * @throw Database_OpenFailed
		 */
		Database(Database::Engine type, const std::string& host, unsigned short port, const std::string& user, const std::string& pass, const std::string& db, bool connectnow = true);

		/**
		 * Opens a connection to a database
		 * @throw Database_OpenFailed
		 */
		void Connect(Database::Engine type, const std::string& host, unsigned short port, const std::string& user, const std::string& pass, const std::string& db);

		/**
		 * Disconnects from the database
		 */
		void Close();

		/**
		 * Executes a raw query and returns it's result. Reconnects to the database if required
		 * @throw Database_QueryFailed
		 * @throw Database_OpenFailed
		 */
		Database_Result RawQuery(const char* query, bool tx_control = false);

		/**
		 * Executes a formatted query and returns it's result. Reconnects to the database if required
		 * @throw Database_QueryFailed
		 * @throw Database_OpenFailed
		 */
		Database_Result Query(const char *format, ...);

		/**
		 * Escapes a piece of text (including Query replacement tokens)
		 */
		std::string Escape(const std::string&);

		/**
		 * Executes a set of queries, rolling back the result of any previous queries if one fails.
		 * @throw Database_QueryFailed
		 * @throw Database_OpenFailed
		 */
		template <class IT> void ExecuteQueries(IT begin, IT end)
		{
			using namespace std::placeholders;

			Bulk_Query_Context qc(*this);

			if (!qc.Pending())
				throw Database_Exception("Cannot execute bulk query with transaction in progress");

			std::for_each(begin, end, std::bind(&Bulk_Query_Context::RawQuery, &qc, _1));

			qc.Commit();
		}

		/**
		 * Executes the contents of a multi-sql command file separated with semicolons.
		 * See ExecuteQueries() for more information.
		 * @throw Database_QueryFailed
		 * @throw Database_OpenFailed
		 */
		void ExecuteFile(const std::string& filename);

		bool Pending() const;
		bool BeginTransaction();
		void Commit();
		void Rollback();

		/**
		 * Closes the database connection if one is active
		 */
		~Database();

		/**
		 * Object used to collect information from an external callback
		 */
		Database_Result callbackdata;
};

#endif // DATABASE_HPP_INCLUDED
