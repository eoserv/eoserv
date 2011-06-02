
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include <functional>

#include "character.hpp"
#include "npc.hpp"

namespace Handlers
{

static void walk_common(Character *character, PacketReader &reader, bool (Character::*f)(Direction))
{
	Direction direction = static_cast<Direction>(reader.GetChar());
	/*int timestamp = */reader.GetThree();
	unsigned char x = reader.GetChar();
	unsigned char y = reader.GetChar();

	if (character->sitting != SIT_STAND)
	{
		return;
	}

	if (direction >= 0 && direction <= 3)
	{
		character->npc = 0;
		character->npc_type = ENF::NPC;
		character->board = 0;
		character->jukebox_open = false;

		if (character->trading)
		{
			PacketBuilder builder(PACKET_TRADE, PACKET_CLOSE, 2);
			builder.AddShort(character->id);
			character->trade_partner->Send(builder);

			character->trading = false;
			character->trade_inventory.clear();
			character->trade_agree = false;

			character->trade_partner->trading = false;
			character->trade_partner->trade_inventory.clear();
			character->trade_agree = false;

			character->trade_partner->trade_partner = 0;
			character->trade_partner = 0;
		}

		if (!(character->*f)(direction))
		{
			return;
		}
	}

	if (character->x != x || character->y != y)
	{
		character->Refresh();
	}
}

// Player walking (admin)
void Walk_Admin(Character *character, PacketReader &reader)
{
	if (character->admin < ADMIN_GUARDIAN)
		return;

	walk_common(character, reader, &Character::AdminWalk);
}

// Player walking (normal)
void Walk_Player(Character *character, PacketReader &reader)
{
	walk_common(character, reader, &Character::Walk);
}

// Player walking (ghost)
/*void Walk_Spec(Character *character, PacketReader &reader)
{
	walk_common(character, reader, &Character::Walk);
}*/

PACKET_HANDLER_REGISTER(PACKET_WALK)
	Register(PACKET_ADMIN, Walk_Admin, Playing, 0.46);
	Register(PACKET_PLAYER, Walk_Player, Playing, 0.46);
	Register(PACKET_SPEC, Walk_Player, Playing, 0.46);
PACKET_HANDLER_REGISTER_END()

}
