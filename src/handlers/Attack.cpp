
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

#include "map.hpp"

CLIENT_F_FUNC(Attack)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_USE: // Player attacking
		{
			if (this->state < EOClient::Playing) return false;
			CLIENT_QUEUE_ACTION(0.58)

			Direction direction = static_cast<Direction>(reader.GetChar());
			/*int timestamp = */reader.GetThree();

			if (this->player->character->sitting != SIT_STAND)
			{
				return true;
			}

			if (direction != this->player->character->direction)
			{
				if (direction >= 0 && direction <= 3)
				{
					this->player->character->map->Face(this->player->character, direction);
					CLIENT_FORCE_QUEUE_ACTION(0.67)
				}
				else
				{
					return false;
				}
			}

			this->player->character->Attack(direction);
		}
		break;

		default:
			return false;
	}

	return true;
}
