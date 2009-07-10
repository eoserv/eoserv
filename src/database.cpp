
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "database.hpp"

#include <string>
#include <map>
#include <stdexcept>
#include <cstdarg>
#include <cstdio>

#include "util.hpp"

int sqlite_callback(void *data, int num, char *fields[], char *columns[])
{
	std::map<std::string, util::variant> result;
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

	((Database *)data)->callbackdata.push_back(result);
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

Database::Database()
{
	this->engine = (Engine)0;
	this->connected = false;
}

Database::Database(Database::Engine type, std::string host, std::string user, std::string pass, std::string db, bool connectnow)
{
	this->connected = false;

	this->engine = type;
	this->host = host;
	this->user = user;
	this->pass = pass;
	this->db = db;

	if (connectnow)
	{
		this->Connect(type, host, user, pass, db);
	}
}

void Database::Connect(Database::Engine type, std::string host, std::string user, std::string pass, std::string db)
{
	this->engine = type;
	this->host = host;
	this->user = user;
	this->pass = pass;
	this->db = db;

	if (this->connected)
	{
		return;
	}

	switch (type)
	{
#ifdef DATABASE_MYSQL
		case MySQL:
			if ((this->mysql_handle = mysql_init(0)) == 0)
			{
				throw Database_OpenFailed(mysql_error(this->mysql_handle));
			}
			if (mysql_real_connect(this->mysql_handle, host.c_str(), user.c_str(), pass.c_str(), 0, 0, 0, 0) != this->mysql_handle)
			{
				throw Database_OpenFailed(mysql_error(this->mysql_handle));
			}
			if (mysql_select_db(this->mysql_handle, db.c_str()) != 0)
			{
				throw Database_OpenFailed(mysql_error(this->mysql_handle));
			}
			this->connected = true;
			break;
#endif // DATABASE_MYSQL

#ifdef DATABASE_SQLITE
		case SQLite:
			if (sqlite3_open(host.c_str(), &this->sqlite_handle) != SQLITE_OK)
			{
				throw Database_OpenFailed(sqlite3_errmsg(this->sqlite_handle));
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
			mysql_close(this->mysql_handle);
			break;
#endif // DATABASE_MYSQL

#ifdef DATABASE_SQLITE
		case SQLite:
			sqlite3_close(this->sqlite_handle);
			break;
#endif // DATABASE_SQLITE
	}
}

Database_Result Database::Query(const char *format, ...)
{
	if (!this->connected)
	{
		this->Connect(this->engine, this->host, this->user, this->pass, this->db);
	}

	va_list ap;
	va_start(ap, format);
	std::string finalquery;
	int tempi;
	char *tempc;
	char *escret;
	Database_Result result;

	for (const char *p = format; *p != '\0'; ++p)
	{
		if (*p == '#'){
			tempi = va_arg(ap,int);
			finalquery += util::to_string(tempi);
		} else if (*p == '@'){
			tempc = va_arg(ap,char *);
			finalquery += static_cast<std::string>(tempc);
		} else if (*p == '$'){
			tempc = va_arg(ap,char *);
			switch (this->engine)
			{
#ifdef DATABASE_MYSQL
				case MySQL:
					tempi = strlen(tempc);
					escret = new char[tempi*2+1];
					mysql_real_escape_string(this->mysql_handle, escret, tempc, tempi);
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
		} else {
			finalquery += *p;
		}
	}
	va_end(ap);

#ifdef DATABASE_DEBUG
	std::puts(finalquery.c_str());
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
			if (mysql_real_query(this->mysql_handle, finalquery.c_str(), finalquery.length()) != 0)
			{
				int myerr = mysql_errno(this->mysql_handle);
				if (myerr == CR_SERVER_GONE_ERROR || myerr == CR_SERVER_LOST)
				{
					this->Close();
					this->Connect(this->engine, this->host, this->user, this->pass, this->db);
					goto exec_query;
				}
				else
				{
					throw Database_QueryFailed(mysql_error(this->mysql_handle));
				}
			}

			num_fields = mysql_field_count(this->mysql_handle);

			if ((mresult = mysql_store_result(this->mysql_handle)) == 0)
			{
				if (num_fields == 0)
				{
					result.affected_rows = mysql_affected_rows(this->mysql_handle);
					return result;
				}
				else
				{
					throw Database_QueryFailed(mysql_error(this->mysql_handle));
				}
			}

			fields = mysql_fetch_fields(mresult);

			result.resize(mysql_num_rows(mresult));
			int i = 0;
			for (MYSQL_ROW row = mysql_fetch_row(mresult); row != 0; row = mysql_fetch_row(mresult))
			{
				std::map<std::string, util::variant> resrow;
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
			if (sqlite3_exec(this->sqlite_handle, finalquery.c_str(), sqlite_callback, (void *)this, 0) != SQLITE_OK)
			{
				throw Database_QueryFailed(sqlite3_errmsg(this->sqlite_handle));
			}
			result = this->callbackdata;
			this->callbackdata.clear();
			break;
#endif // DATABASE_SQLITE
	}

	return result;
}

std::string Database::Escape(std::string raw)
{
	char *escret;

	switch (this->engine)
	{
#ifdef DATABASE_MYSQL
		case MySQL:
			escret = new char[raw.length()*2+1];
			mysql_real_escape_string(this->mysql_handle, escret, raw.c_str(), raw.length());
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
		if (*it == '@' || *it == '#' || *it == '%')
		{
			*it = '?';
		}
	}

	return raw;
}

Database::~Database()
{
	this->Close();
}
