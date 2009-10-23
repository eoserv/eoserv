
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

#include "map.hpp"

CLIENT_F_FUNC(Door)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_OPEN: // User opening a door
		{
			if (this->state < EOClient::Playing) return false;

			int x = reader.GetChar();
			int y = reader.GetChar();

			this->player->character->map->OpenDoor(this->player->character, x, y);
		}
		break;

		default:
			return false;
	}

	return true;
}
