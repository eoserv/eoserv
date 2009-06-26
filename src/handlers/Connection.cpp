
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

CLIENT_F_FUNC(Connection)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_ACCEPT: // Reply after PACKET_INIT transfer is complete
		{

		}
		break;

		case PACKET_NET: // Response to a PACKET_PING from the server
		{
			this->needpong = false;
		}
		break;

		default:
			return false;
	}

	return true;
}
