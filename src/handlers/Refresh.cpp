
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

CLIENT_F_FUNC(Refresh)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // User requesting data of all objects in their location
		{
			if (this->state < EOClient::PlayingModal) return false;

			this->player->character->Refresh();
		}
		break;

		default:
			return false;
	}

	return true;
}
