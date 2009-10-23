
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

CLIENT_F_FUNC(Attack)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_USE: // Player attacking
		{
			if (this->state < EOClient::Playing) return false;
			CLIENT_QUEUE_ACTION(0.6)

			Direction direction = static_cast<Direction>(reader.GetChar());
			/*int timestamp = */reader.GetThree();

			if (this->player->character->sitting != SIT_STAND)
			{
				return true;
			}

			if (direction >= 0 && direction <= 3)
			{
				this->player->character->Attack(direction);
			}
		}
		break;

		default:
			return false;
	}

	return true;
}
