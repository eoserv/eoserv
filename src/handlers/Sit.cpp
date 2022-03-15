/* handlers/Sit.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../packet.hpp"

namespace Handlers
{

void Sit_Request(Character *character, PacketReader &reader)
{
	SitAction action = SitAction(reader.GetChar());

	if (action == SIT_ACT_SIT && character->sitting == SIT_STAND)
	{
		PacketBuilder reply(PACKET_SIT, PACKET_PLAYER, 6);
		reply.AddShort(character->PlayerID());
		reply.AddChar(character->x);
		reply.AddChar(character->y);
		reply.AddChar(character->direction);
		reply.AddChar(0); // ?
		character->Send(reply);
		character->Sit(SIT_FLOOR);
	}
	else if (character->sitting == SIT_FLOOR)
	{
		PacketBuilder reply(PACKET_SIT, PACKET_CLOSE, 4);
		reply.AddShort(character->PlayerID());
		reply.AddChar(character->x);
		reply.AddChar(character->y);
		character->Send(reply);
		character->Stand();
	}
}

PACKET_HANDLER_REGISTER(PACKET_SIT)
	Register(PACKET_REQUEST, Sit_Request, Playing);
PACKET_HANDLER_REGISTER_END(PACKET_SIT)

}
