
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

CLIENT_F_FUNC(Ping)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_NET: // User sending a ping request (#ping)
		{
			if (this->state < EOClient::PlayingModal) return false;

			int something = reader.GetShort();

			reply.SetID(PACKET_PING, PACKET_NET2);
			reply.AddShort(something);
			CLIENT_SEND(reply);
		}
		break;

		default:
			return false;
	}

	return true;
}
