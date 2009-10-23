
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

CLIENT_F_FUNC(Jukebox)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_OPEN: // Opened the jukebox listing
		{

		}
		break;

		case PACKET_MSG: // Requested a song
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
