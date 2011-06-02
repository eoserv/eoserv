
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef FWD_CONSOLE_HPP_INCLUDED
#define FWD_CONSOLE_HPP_INCLUDED

#include <string>

namespace Console
{

void Out(std::string f, ...);
void Wrn(std::string f, ...);
void Err(std::string f, ...);
void Dbg(std::string f, ...);

};

#endif // FWD_CONFIG_HPP_INCLUDED
