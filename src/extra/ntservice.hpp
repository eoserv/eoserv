
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#if !defined(WIN32) && !defined(WIN64)
#error Services are Windows only
#endif // !defined(WIN32) && !defined(WIN64)

void service_init(const char *name);
bool service_install(const char *name);
bool service_uninstall(const char *name);
