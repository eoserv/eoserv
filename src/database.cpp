
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "database.hpp"

#include <cstdarg>
#include <cstring>

#include "console.hpp"
#include "util.hpp"

#include "database_impl.hpp"

#ifndef DATABASE_MYSQL
#ifndef DATABASE_SQLITE
#error At least one database driver must be selected
#endif // DATABASE_SQLITE
#endif // DATABASE_MYSQL

struct Database::impl_
{
	union
	{
#ifdef DATABASE_MYSQL
		MYSQL *mysql_handle;
#endif // DATABASE_MYSQL
#ifdef DATABASE_SQLITE
		sqlite3 *sqlite_handle;
#endif // DATABASE_SQLITE
	};
};

static int sqlite_callback(void *data, int num, char *fields[], char *columns[])
{
	std::unordered_map<std::string, util::variant> result;
	std::string column;
	util::variant field;
	int i;

	for (i = 0; i < num; ++i)
	{
		if (columns[i] == NULL)
		{
			column = "";
		}
		else
		{
			column = columns[i];
		}

		if (fields[i] == NULL)
		{
			field = "";
		}
		else
		{
			field = fields[i];
		}

		result.insert(result.begin(), make_pair(column, field));
	}

	static_cast<Database *>(data)->callbackdata.push_back(result);
	return 0;
}

int Database_Result::AffectedRows()
{
	return this->affected_rows;
}

bool Database_Result::Error()
{
	return this->error;
}

Database::Bulk_Query_Context::Bulk_Query_Context(Database& db)
	: db(db)
	, pending(false)
{
	db.BeginTransaction();
	pending = true;
}

void Database::Bulk_Query_Context::RawQuery(const std::string& query)
{
	db.RawQuery(query.c_str());
}

void Database::Bulk_Query_Context::Commit()
{
	if (pending)
		db.Commit();

	pending = false;
}

void Database::Bulk_Query_Context::Rollback()
{
	if (pending)
		db.Rollback();

	pending = false;
}

Database::Bulk_Query_Context::~Bulk_Query_Context()
{
	if (pending)
		db.Rollback();
}

Database::Database()
	: impl(new impl_)
	, connected(false)
	, engine(Engine(0))
{ }

Database::Database(Database::Engine type, std::string host, unsigned short port, std::string user, std::string pass, std::string db, bool connectnow)
{
	this->connected = false;

	this->engine = type;
	this->host = host;
	this->user = user;
	this->pass = pass;
	this->port = port;
	this->db = db;

	if (connectnow)
	{
		this->Connect(type, host, port, user, pass, db);
	}
}

void Database::Connect(Database::Engine type, std::string host, unsigned short port, std::string user, std::string pass, std::string db)
{
	this->engine = type;
	this->host = host;
	this->user = user;
	this->pass = pass;
	this->port = port;
	this->db = db;

	if (this->connected)
	{
		return;
	}

	switch (type)
	{
#ifdef DATABASE_MYSQL
		case MySQL:
			if ((this->impl->mysql_handle = mysql_init(0)) == 0)
			{
				throw Database_OpenFailed(mysql_error(this->impl->mysql_handle));
			}
			if (mysql_real_connect(this->impl->mysql_handle, host.c_str(), user.c_str(), pass.c_str(), 0, this->port, 0, 0) != this->impl->mysql_handle)
			{
				throw Database_OpenFailed(mysql_error(this->impl->mysql_handle));
			}
			if (mysql_select_db(this->impl->mysql_handle, db.c_str()) != 0)
			{
				throw Database_OpenFailed(mysql_error(this->impl->mysql_handle));
			}

			this->connected = true;

			break;
#endif // DATABASE_MYSQL

#ifdef DATABASE_SQLITE
		case SQLite:
			if (sqlite3_open(host.c_str(), &this->impl->sqlite_handle) != SQLITE_OK)
			{
				throw Database_OpenFailed(sqlite3_errmsg(this->impl->sqlite_handle));
			}

			this->connected = true;

			break;
#endif // DATABASE_SQLITE

		default:
			throw Database_OpenFailed("Invalid database engine.");
	}
}

void Database::Close()
{
	if (!this->connected)
	{
		return;
	}

	this->connected = false;

	switch (this->engine)
	{
#ifdef DATABASE_MYSQL
		case MySQL:
			mysql_close(this->impl->mysql_handle);
			break;
#endif // DATABASE_MYSQL

#ifdef DATABASE_SQLITE
		case SQLite:
			sqlite3_close(this->impl->sqlite_handle);
			break;
#endif // DATABASE_SQLITE
	}
}

Database_Result Database::RawQuery(const char* query)
{
	std::size_t query_length = std::strlen(query);

	Database_Result result;

#ifdef DATABASE_DEBUG
	Console::Dbg(query);
#endif // DATABASE_DEBUG

	switch (this->engine)
	{
#ifdef DATABASE_MYSQL
		case MySQL:
		{
			MYSQL_RES *mresult;
			MYSQL_FIELD *fields;
			int num_fields;

			exec_query:
			if (mysql_real_query(this->impl->mysql_handle, query, query_length) != 0)
			{
				int myerr = mysql_errno(this->impl->mysql_handle);

				if (myerr == CR_SERVER_GONE_ERROR || myerr == CR_SERVER_LOST)
				{
					this->Close();
					this->Connect(this->engine, this->host, this->port, this->user, this->pass, this->db);
					goto exec_query;
				}
				else
				{
					throw Database_QueryFailed(mysql_error(this->impl->mysql_handle));
				}
			}

			num_fields = mysql_field_count(this->impl->mysql_handle);

			if ((mresult = mysql_store_result(this->impl->mysql_handle)) == 0)
			{
				if (num_fields == 0)
				{
					result.affected_rows = mysql_affected_rows(this->impl->mysql_handle);
					return result;
				}
				else
				{
					throw Database_QueryFailed(mysql_error(this->impl->mysql_handle));
				}
			}

			fields = mysql_fetch_fields(mresult);

			result.resize(mysql_num_rows(mresult));
			int i = 0;
			for (MYSQL_ROW row = mysql_fetch_row(mresult); row != 0; row = mysql_fetch_row(mresult))
			{
				std::unordered_map<std::string, util::variant> resrow;
				for (int ii = 0; ii < num_fields; ++ii)
				{
					util::variant rescell;
					if (IS_NUM(fields[ii].type))
					{
						if (row[ii])
						{
							rescell = util::to_int(row[ii]);
						}
						else
						{
							rescell = 0;
						}
					}
					else
					{
						if (row[ii])
						{
							rescell = row[ii];
						}
						else
						{
							rescell = "";
						}
					}
					resrow[fields[ii].name] = rescell;
				}
				result[i++] = resrow;
			}

			mysql_free_result(mresult);
		}
		break;
#endif // DATABASE_MYSQL

#ifdef DATABASE_SQLITE
		case SQLite:
			if (sqlite3_exec(this->impl->sqlite_handle, query, sqlite_callback, (void *)this, 0) != SQLITE_OK)
			{
				throw Database_QueryFailed(sqlite3_errmsg(this->impl->sqlite_handle));
			}
			result = this->callbackdata;
			this->callbackdata.clear();
			break;
#endif // DATABASE_SQLITE

		default:
			throw Database_QueryFailed("Unknown database engine");
	}

	return result;
}

Database_Result Database::Query(const char *format, ...)
{
	if (!this->connected)
	{
		this->Connect(this->engine, this->host, this->port, this->user, this->pass, this->db);
	}

	std::va_list ap;
	va_start(ap, format);

	std::string finalquery;
	int tempi;
	char *tempc;
	char *escret;

	for (const char *p = format; *p != '\0'; ++p)
	{
		if (*p == '#')
		{
			tempi = va_arg(ap,int);
			finalquery += util::to_string(tempi);
		}
		else if (*p == '@')
		{
			tempc = va_arg(ap,char *);
			finalquery += static_cast<std::string>(tempc);
		}
		else if (*p == '$')
		{
			tempc = va_arg(ap,char *);
			switch (this->engine)
			{
#ifdef DATABASE_MYSQL
				case MySQL:
					tempi = strlen(tempc);
					escret = new char[tempi*2+1];
					mysql_real_escape_string(this->impl->mysql_handle, escret, tempc, tempi);
					finalquery += escret;
					delete[] escret;
					break;
#endif // DATABASE_MYSQL

#ifdef DATABASE_SQLITE
				case SQLite:
					escret = sqlite3_mprintf("%q",tempc);
					finalquery += escret;
					sqlite3_free(escret);
					break;
#endif // DATABASE_SQLITE
			}
		}
		else
		{
			finalquery += *p;
		}
	}

	va_end(ap);

	return this->RawQuery(finalquery.c_str());
}

std::string Database::Escape(std::string raw)
{
	char *escret;

	switch (this->engine)
	{
#ifdef DATABASE_MYSQL
		case MySQL:
			escret = new char[raw.length()*2+1];
			mysql_real_escape_string(this->impl->mysql_handle, escret, raw.c_str(), raw.length());
			raw = escret;
			delete[] escret;
			break;
#endif // DATABASE_MYSQL

#ifdef DATABASE_SQLITE
		case SQLite:
			escret = sqlite3_mprintf("%q", raw.c_str());
			raw = escret;
			sqlite3_free(escret);
			break;
#endif // DATABASE_SQLITE
	}

	for (std::string::iterator it = raw.begin(); it != raw.end(); ++it)
	{
		if (*it == '@' || *it == '#' || *it == '$')
		{
			*it = '?';
		}
	}

	return raw;
}

void Database::ExecuteFile(std::string filename)
{
	std::list<std::string> queries;
	std::string query;

	FILE* fh = std::fopen(filename.c_str(), "rt");

	try
	{
		while (true)
		{
			char buf[4096];

			const char *result = std::fgets(buf, 4096, fh);

			if (!result || std::feof(fh))
				break;

			for (char* p = buf; *p != '\0'; ++p)
			{
				if (*p == ';')
				{
					queries.push_back(query);
					query.erase();
				}
				else
				{
					query += *p;
				}
			}
		}
	}
	catch (std::exception &e)
	{
		std::fclose(fh);
		throw;
	}

	std::fclose(fh);

	queries.push_back(query);
	query.erase();

	std::remove_if(UTIL_RANGE(queries), [&](const std::string& s) { return util::trim(s).length() == 0; });

	this->ExecuteQueries(UTIL_RANGE(queries));
}

void Database::BeginTransaction()
{
	switch (this->engine)
	{
#ifdef DATABASE_MYSQL
		case MySQL:
			this->RawQuery("START TRANSACTION");
			break;
#endif // DATABASE_MYSQL

#ifdef DATABASE_SQLITE
		case SQLite:
			this->RawQuery("BEGIN");
			break;
#endif // DATABASE_SQLITE
	}
}

void Database::Commit()
{
	this->RawQuery("COMMIT");
}

void Database::Rollback()
{
	this->RawQuery("ROLLBACK");
}

Database::~Database()
{
	this->Close();
}
