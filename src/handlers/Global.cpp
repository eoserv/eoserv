
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

namespace Handlers
{

// User has opened/closed the global tab
void Global_Open(Character *character, PacketReader &reader)
{
	(void)character;
	(void)reader;
}

// User has closed the global tab
void Global_Close(Character *character, PacketReader &reader)
{
	(void)character;
	(void)reader;
}

PACKET_HANDLER_REGISTER(PACKET_GLOBAL)
	Register(PACKET_OPEN, Global_Open, Playing);
	Register(PACKET_CLOSE, Global_Close, Playing);
PACKET_HANDLER_REGISTER_END()

}
