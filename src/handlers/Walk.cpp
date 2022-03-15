/* handlers/Walk.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../config.hpp"
#include "../eodata.hpp"
#include "../npc.hpp"
#include "../packet.hpp"
#include "../world.hpp"

namespace Handlers
{

static void walk_common(Character *character, PacketReader &reader, Map::WalkResult (Character::*f)(Direction))
{
	Direction direction = static_cast<Direction>(reader.GetChar());
	Timestamp timestamp = reader.GetThree();
	unsigned char x = reader.GetChar();
	unsigned char y = reader.GetChar();
	Map::WalkResult walk_result = Map::WalkFail;

	if (character->world->config["EnforceTimestamps"])
	{
		if (timestamp - character->timestamp < 36)
		{
			return;
		}
	}

	character->timestamp = timestamp;

	if (character->sitting != SIT_STAND)
	{
		return;
	}

	if (direction <= 3)
	{
		character->npc = 0;
		character->npc_type = ENF::NPC;
		character->board = 0;
		character->jukebox_open = false;

		if (character->trading)
		{
			PacketBuilder builder(PACKET_TRADE, PACKET_CLOSE, 2);
			builder.AddShort(character->PlayerID());
			character->trade_partner->Send(builder);

			character->trading = false;
			character->trade_inventory.clear();
			character->trade_agree = false;

			character->trade_partner->trading = false;
			character->trade_partner->trade_inventory.clear();
			character->trade_agree = false;

			character->CheckQuestRules();
			character->trade_partner->CheckQuestRules();

			character->trade_partner->trade_partner = 0;
			character->trade_partner = 0;
		}

		walk_result = (character->*f)(direction);
	}

	if (walk_result == Map::WalkFail || (walk_result == Map::WalkOK && (character->x != x || character->y != y)))
	{
		character->Refresh();
	}
}

// Player walking (admin)
void Walk_Admin(Character *character, PacketReader &reader)
{
	if (character->SourceDutyAccess() < int(character->world->admin_config["nowall"]))
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
	Register(PACKET_ADMIN, Walk_Admin, Playing, 0.4);
	Register(PACKET_PLAYER, Walk_Player, Playing, 0.4);
	Register(PACKET_SPEC, Walk_Player, Playing, 0.4);
PACKET_HANDLER_REGISTER_END(PACKET_WALK)

}
