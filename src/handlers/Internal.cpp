/* handlers/Internal.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"

namespace Handlers
{

void Internal_Null(EOClient *client, PacketReader &reader)
{
	(void)client;
	(void)reader;
}

// Death warp
void Internal_Warp(Character *character, PacketReader &reader)
{
	(void)reader;
	character->map = 0;
	character->Warp(character->SpawnMap(), character->SpawnX(), character->SpawnY(), WARP_ANIMATION_NONE);
}

PACKET_HANDLER_REGISTER(PACKET_INTERNAL)
	Register(PACKET_INTERNAL_NULL, Internal_Null, Any);
	Register(PACKET_INTERNAL_WARP, Internal_Warp, Playing);
PACKET_HANDLER_REGISTER_END(PACKET_INTERNAL)

}
