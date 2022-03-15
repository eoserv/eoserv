/* handlers/Refresh.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"

namespace Handlers
{

// User requesting data of all objects in their location
void Refresh_Request(Character *character, PacketReader &reader)
{
	(void)reader;

	character->Refresh();
}

PACKET_HANDLER_REGISTER(PACKET_REFRESH)
	Register(PACKET_REQUEST, Refresh_Request, Playing);
PACKET_HANDLER_REGISTER_END(PACKET_REFRESH)

}
