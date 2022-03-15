/* handlers/Message.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../packet.hpp"

namespace Handlers
{

// User sending a ping request (#ping)
void Message_Ping(Character *character, PacketReader &reader)
{
	int something = reader.GetShort();

	PacketBuilder reply(PACKET_MESSAGE, PACKET_PONG, 2);
	reply.AddShort(something);
	character->Send(reply);
}

PACKET_HANDLER_REGISTER(PACKET_MESSAGE)
	Register(PACKET_PING, Message_Ping, Playing | OutOfBand)
PACKET_HANDLER_REGISTER_END(PACKET_MESSAGE)

}
