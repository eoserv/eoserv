/* console.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef CONSOLE_HPP_INCLUDED
#define CONSOLE_HPP_INCLUDED

#include "fwd/console.hpp"

#include <string>

namespace Console
{

extern bool Styled[2];

enum Color
{
	COLOR_BLUE = 1,
	COLOR_GREEN = 2,
	COLOR_CYAN = 3,
	COLOR_RED = 4,
	COLOR_MAGENTA = 5,
	COLOR_YELLOW = 6,
	COLOR_GREY = 7,
	COLOR_BLACK = 8
};

enum Stream
{
	STREAM_OUT,
	STREAM_ERR
};

void Out(const char* f, ...);
void Wrn(const char* f, ...);
void Err(const char* f, ...);
void Dbg(const char* f, ...);

}

#endif // CONSOLE_HPP_INCLUDED
