/* extra/ntservice.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef EXTRA_NTSERVICE_HPP_INCLUDED
#define EXTRA_NTSERVICE_HPP_INCLUDED

#include "../platform.h"

#ifndef WIN32
#error Services are Windows only
#endif // WIN32

void service_init(const char *name);
bool service_install(const char *name);
bool service_uninstall(const char *name);

#endif // EXTRA_NTSERVICE_HPP_INCLUDED
