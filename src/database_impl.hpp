
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef DATABASE_IMPL_HPP_INCLUDED
#define DATABASE_IMPL_HPP_INCLUDED

#ifdef DATABASE_MYSQL
#include "socket_impl.hpp"
#include <mysql.h>
#include <errmsg.h>
#endif // DATABASE_MYSQL
#ifdef DATABASE_SQLITE
#include <sqlite3.h>
#endif // DATABASE_SQLITE

#endif // DATABASE_IMPL_HPP_INCLUDED
