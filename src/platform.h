/* platform.h
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef PLATFORM_H_INCLUDED
#define PLATFORM_H_INCLUDED

#if defined(_WIN32) || defined(EOSERV_MINGW)
#ifndef WIN32
#define WIN32
#endif // WIN32
#endif // defined(_WIN32) || defined(EOSERV_MINGW)

#endif // PLATFORM_H_INCLUDED
