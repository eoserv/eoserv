
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

CLIENT_F_FUNC(Citizen)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REMOVE: // Player giving up citizenship of a town
		{

		}
		break;

		case PACKET_OPEN: // Talked to a citizenship NPC
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
