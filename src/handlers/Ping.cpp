
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "character.hpp"

namespace Handlers
{

// User sending a ping request (#ping)
void Ping_Net(Character *character, PacketReader &reader)
{
	int something = reader.GetShort();

	PacketBuilder reply(PACKET_PING, PACKET_NET2);
	reply.AddShort(something);
	character->Send(reply);
}

PACKET_HANDLER_REGISTER(PACKET_PING)
	Register(PACKET_NET, Ping_Net, Playing | OutOfBand)
PACKET_HANDLER_REGISTER_END()

}
