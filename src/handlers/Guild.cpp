
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

CLIENT_F_FUNC(Guild)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_OPEN: // Talked to a guild NPC
		{

		}
		break;

		case PACKET_TELL: // Requested member list of a guild
		{

		}
		break;

		case PACKET_REPORT: // Requested information on a guild
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
