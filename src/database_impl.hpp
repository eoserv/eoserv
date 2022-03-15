/* database_impl.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef DATABASE_IMPL_HPP_INCLUDED
#define DATABASE_IMPL_HPP_INCLUDED

#ifdef DATABASE_MYSQL
#include "socket_impl.hpp"

// Define this for broken MariaDB C connector release 2.1.0
#ifdef MARIADB_CC_2_1_0_WORKAROUND
#if defined(__EMX__) || !defined(HAVE_UINT)
typedef unsigned int uint;
#endif
#endif // MARIADB_CC_2_1_0_WORKAROUND

#include <mysql.h>
#include <errmsg.h>
#endif // DATABASE_MYSQL

#ifdef DATABASE_SQLITE
#include <sqlite3.h>
#endif // DATABASE_SQLITE

#endif // DATABASE_IMPL_HPP_INCLUDED
