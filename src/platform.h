
/* $Id$
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

// Allow compilation on GCC 4.5
#ifdef __GNUC__
#if __GNUC__ == 4 && __GNUC_MINOR__ <= 5
#define noexcept throw()
#endif // __GNUC__ == 4 && __GNUC_MINOR__ <= 5
#endif // __GNUC__

#endif // PLATFORM_H_INCLUDED
