
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

CLIENT_F_FUNC(Locker)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_ADD: // Placing an item in a bank locker
		{

		}
		break;

		case PACKET_TAKE: // Taking an item from a bank locker
		{

		}
		break;

		case PACKET_OPEN: // Opening a bank locker
		{

		}
		break;

		case PACKET_BUY: // Purchasing a locker space upgrade
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
