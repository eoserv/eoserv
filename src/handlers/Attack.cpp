
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
			if (!act) printf("%.4g\n", Timer::GetTime());
			CLIENT_QUEUE_ACTION(0.58)

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
