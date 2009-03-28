
#include "database.hpp"

#include <string>
#include <stdexcept>
#include <cstdarg>

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

Database::Database(Database::Engine type, std::string host, std::string user, std::string pass, std::string db)
{
	this->connected = false;
	this->Connect(type, host, user, pass, db);
}

void Database::Connect(Database::Engine type, std::string host, std::string user, std::string pass, std::string db)
{
	this->engine = type;
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
			break;
#endif // DATABASE_MYSQL

#ifdef DATABASE_SQLITE
		case SQLite:
			if (sqlite3_open(host.c_str(), &this->sqlite_handle) != SQLITE_OK)
			{
				throw Database_OpenFailed("sqlite3err");
			}
			break;
#endif // DATABASE_SQLITE

		default:
			throw Database_Exception("Invalid database engine.");
	}
}

Database_Result Database::Query(const char *format, ...)
{
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
			finalquery += static_cast<std::string>(util::variant(tempi));
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
					delete escret;
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

			if (mysql_real_query(this->mysql_handle, finalquery.c_str(), finalquery.length()) != 0)
			{
				throw Database_QueryFailed(mysql_error(this->mysql_handle));
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

			for (MYSQL_ROW row = mysql_fetch_row(mresult); row != 0; row = mysql_fetch_row(mresult))
			{
				std::map<std::string, util::variant> resrow;
				for (int i = 0; i < num_fields; ++i)
				{
					util::variant rescell;
					if (IS_NUM(fields[i].type))
					{
						if (row[i])
						{
							rescell = static_cast<int>(util::variant(row[i]));
						}
						else
						{
							rescell = 0;
						}
					}
					else
					{
						if (row[i])
						{
							rescell = row[i];
						}
						else
						{
							rescell = "";
						}
					}
					resrow[fields[i].name] = rescell;
				}
				result.push_back(resrow);
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

