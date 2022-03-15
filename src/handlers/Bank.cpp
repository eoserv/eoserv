/* handlers/Bank.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../config.hpp"
#include "../eodata.hpp"
#include "../map.hpp"
#include "../npc.hpp"
#include "../packet.hpp"
#include "../world.hpp"

#include "../util.hpp"

#include <algorithm>

// TODO: Correct overflow checking

namespace Handlers
{

// Talked to a banker NPC
void Bank_Open(Character *character, PacketReader &reader)
{
	short id = reader.GetShort();

	UTIL_FOREACH(character->map->npcs, npc)
	{
		if (npc->index == id && npc->ENF().type == ENF::Bank)
		{
			character->npc = npc;
			character->npc_type = ENF::Bank;

			PacketBuilder reply(PACKET_BANK, PACKET_OPEN, 8);
			reply.AddInt(character->goldbank);
			reply.AddThree(0); // Session token
			reply.AddChar(character->bankmax);

			character->Send(reply);

			break;
		}
	}
}

// Depositing gold
void Bank_Add(Character *character, PacketReader &reader)
{
	if (character->trading) return;

	int amount = reader.GetInt();

	if (amount <= 0) return;

	amount = std::min(amount, character->HasItem(1));

	if (character->npc_type == ENF::Bank)
	{
		int newgold = character->goldbank + amount;

		if (newgold < character->goldbank || newgold > static_cast<int>(character->world->config["MaxBankGold"]))
		{
			return;
		}

		character->DelItem(1, amount);
		character->goldbank = newgold;

		PacketBuilder reply(PACKET_BANK, PACKET_REPLY, 8);
		reply.AddInt(character->HasItem(1));
		reply.AddInt(character->goldbank);
		character->Send(reply);
	}
}

// Withdrawing gold
void Bank_Take(Character *character, PacketReader &reader)
{
	int amount = reader.GetInt();

	if (amount <= 0) return;

	if (character->npc_type == ENF::Bank)
	{
		int newgold = character->goldbank - amount;

		if (newgold > character->goldbank || newgold < 0)
		{
			return;
		}

		character->goldbank = newgold;
		character->AddItem(1, amount);

		PacketBuilder reply(PACKET_BANK, PACKET_REPLY, 8);
		reply.AddInt(character->HasItem(1));
		reply.AddInt(character->goldbank);
		character->Send(reply);
	}
}

PACKET_HANDLER_REGISTER(PACKET_BANK)
	Register(PACKET_OPEN, Bank_Open, Playing);
	Register(PACKET_ADD, Bank_Add, Playing);
	Register(PACKET_TAKE, Bank_Take, Playing);
PACKET_HANDLER_REGISTER_END(PACKET_BANK)

}
