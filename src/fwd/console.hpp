/* fwd/console.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef FWD_CONSOLE_HPP_INCLUDED
#define FWD_CONSOLE_HPP_INCLUDED

#include <string>

namespace Console
{

void Out(const char* f, ...);
void Wrn(const char* f, ...);
void Err(const char* f, ...);
void Dbg(const char* f, ...);

};

#endif // FWD_CONFIG_HPP_INCLUDED
