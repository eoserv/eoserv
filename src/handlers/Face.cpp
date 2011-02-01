
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

#include "character.hpp"
#include "map.hpp"
#include "player.hpp"

CLIENT_F_FUNC(Face)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_PLAYER: // Player changing direction
		{
			if (this->state < EOClient::Playing) return false;
			CLIENT_QUEUE_ACTION(0.09)

			Direction direction = static_cast<Direction>(reader.GetChar());

			if (this->player->character->sitting != SIT_STAND)
			{
				return true;
			}

			if (direction >= 0 && direction <= 3)
			{
				this->player->character->map->Face(this->player->character, direction);
			}
		}
		break;

		default:
			return false;
	}

	return true;
}
