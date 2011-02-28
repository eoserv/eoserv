
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "character.hpp"
#include "map.hpp"

namespace Handlers
{

// User opening a door
void Door_Open(Character *character, PacketReader &reader)
{
	int x = reader.GetChar();
	int y = reader.GetChar();

	character->map->OpenDoor(character, x, y);
}

PACKET_HANDLER_REGISTER(PACKET_DOOR)
	Register(PACKET_OPEN, Door_Open, Playing);
PACKET_HANDLER_REGISTER_END()

}
