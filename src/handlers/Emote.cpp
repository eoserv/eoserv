/* handlers/Emote.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../packet.hpp"

namespace Handlers
{

// Player sending an emote
void Emote_Report(Character *character, PacketReader &reader)
{
	Emote emote = static_cast<Emote>(reader.GetChar());

	// TODO: Restrict drunk emote
	if ((emote >= 1 && emote <= 10) || emote == EMOTE_DRUNK || emote == EMOTE_PLAYFUL)
	{
		character->Emote(emote, false);
	}
}

PACKET_HANDLER_REGISTER(PACKET_EMOTE)
	Register(PACKET_REPORT, Emote_Report, Playing);
PACKET_HANDLER_REGISTER_END(PACKET_EMOTE)

}
