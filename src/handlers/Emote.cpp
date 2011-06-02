
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "character.hpp"

namespace Handlers
{

// Player sending an emote
void Emote_Report(Character *character, PacketReader &reader)
{
	Emote emote = static_cast<Emote>(reader.GetChar());

	if ((emote >= 0 && emote <= 10) || emote == 12 || emote == 14)
	{
		character->Emote(emote, false);
	}
}

PACKET_HANDLER_REGISTER(PACKET_EMOTE)
	Register(PACKET_REPORT, Emote_Report, Playing);
PACKET_HANDLER_REGISTER_END()

}
