
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

CLIENT_F_FUNC(Emote)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REPORT: // Player sending an emote
		{
			if (this->state < EOClient::PlayingModal) return false;
			CLIENT_QUEUE_ACTION(0.0)

			Emote emote = static_cast<Emote>(reader.GetChar());

			if ((emote >= 0 && emote <= 10) || emote == 14)
			{
				this->player->character->Emote(emote, false);
			}
		}
		break;

		default:
			return false;
	}

	return true;
}
