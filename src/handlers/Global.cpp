
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"

namespace Handlers
{

// User enabled whispers
void Global_Remove(Character *character, PacketReader &reader)
{
	/*char n = */reader.GetChar(); // 'n'

	character->whispers = true;
}

// User disabled whispers
void Global_Player(Character *character, PacketReader &reader)
{
	/*char y = */reader.GetChar(); // 'y'

	character->whispers = false;
}

// User has opened/closed the global tab
void Global_Open(Character *character, PacketReader &reader)
{
	/*char y = */reader.GetChar(); // 'y'

	(void)character;
}

// User has closed the global tab
void Global_Close(Character *character, PacketReader &reader)
{
	/*char n = */reader.GetChar(); // 'n'

	(void)character;
}

PACKET_HANDLER_REGISTER(PACKET_GLOBAL)
	Register(PACKET_REMOVE, Global_Remove, Playing);
	Register(PACKET_PLAYER, Global_Player, Playing);
	Register(PACKET_OPEN, Global_Open, Playing);
	Register(PACKET_CLOSE, Global_Close, Playing);
PACKET_HANDLER_REGISTER_END()

}
