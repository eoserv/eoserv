
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "character.hpp"
#include "eoserver.hpp"
#include "map.hpp"
#include "player.hpp"
#include "world.hpp"

namespace Handlers
{

static PacketBuilder add_common(Character *character, short item, int amount)
{
	character->DelItem(item, amount);

	character->CalculateStats();

	PacketBuilder reply(PACKET_LOCKER, PACKET_REPLY, 8 + character->bank.size() * 5);
	reply.AddShort(item);
	reply.AddInt(character->HasItem(item));
	reply.AddChar(character->weight);
	reply.AddChar(character->maxweight);

	UTIL_FOREACH(character->bank, item)
	{
		reply.AddShort(item.id);
		reply.AddThree(item.amount);
	}

	return reply;
}

// Placing an item in a bank locker
void Locker_Add(Character *character, PacketReader &reader)
{
	if (character->trading) return;

	unsigned char x = reader.GetChar();
	unsigned char y = reader.GetChar();
	short item = reader.GetShort();
	int amount = reader.GetThree();

	if (item == 1) return;
	if (amount <= 0) return;
	if (character->HasItem(item) < amount) return;

	std::size_t lockermax = static_cast<int>(character->world->config["BaseBankSize"]) + character->bankmax * static_cast<int>(character->world->config["BankSizeStep"]);

	if (util::path_length(character->x, character->y, x, y) <= 1)
	{
		if (character->map->GetSpec(x, y) == Map_Tile::BankVault)
		{
			UTIL_IFOREACH(character->bank, it)
			{
				if (it->id == item)
				{
					if (it->amount + amount < 0)
					{
						return;
					}

					amount = std::min<int>(amount, static_cast<int>(character->world->config["MaxBank"]) - it->amount);

					it->amount += amount;

					PacketBuilder reply = add_common(character, item, amount);
					character->Send(reply);
					return;
				}
			}

			if (character->bank.size() >= lockermax)
			{
				return;
			}

			amount = std::min<int>(amount, static_cast<int>(character->world->config["MaxBank"]));

			Character_Item newitem;
			newitem.id = item;
			newitem.amount = amount;

			character->bank.push_back(newitem);

			PacketBuilder reply = add_common(character, item, amount);
			character->Send(reply);
		}
	}
}

// Taking an item from a bank locker
void Locker_Take(Character *character, PacketReader &reader)
{
	if (character->trading) return;

	unsigned char x = reader.GetChar();
	unsigned char y = reader.GetChar();
	short item = reader.GetShort();

	// TODO: Limit number of items withdrawn to under weight

	if (util::path_length(character->x, character->y, x, y) <= 1)
	{
		if (character->map->GetSpec(x, y) == Map_Tile::BankVault)
		{
			UTIL_IFOREACH(character->bank, it)
			{
				if (it->id == item)
				{
					character->AddItem(item, it->amount);

					character->CalculateStats();

					PacketBuilder reply(PACKET_LOCKER, PACKET_GET, 7 + character->bank.size() * 5);
					reply.AddShort(item);
					reply.AddThree(it->amount);
					reply.AddChar(character->weight);
					reply.AddChar(character->maxweight);

					character->bank.erase(it);

					UTIL_FOREACH(character->bank, item)
					{
						reply.AddShort(item.id);
						reply.AddThree(item.amount);
					}
					character->Send(reply);

					break;
				}
			}
		}
	}
}

// Opening a bank locker
void Locker_Open(Character *character, PacketReader &reader)
{
	unsigned char x = reader.GetChar();
	unsigned char y = reader.GetChar();

	if (util::path_length(character->x, character->y, x, y) <= 1)
	{
		if (character->map->GetSpec(x, y) == Map_Tile::BankVault)
		{
			PacketBuilder reply(PACKET_LOCKER, PACKET_OPEN, 2 + character->bank.size() * 5);
			reply.AddChar(x);
			reply.AddChar(y);
			UTIL_FOREACH(character->bank, item)
			{
				reply.AddShort(item.id);
				reply.AddThree(item.amount);
			}
			character->Send(reply);
		}
	}
}

// Purchasing a locker space upgrade
void Locker_Buy(Character *character, PacketReader &reader)
{
	if (character->trading) return;

	(void)reader;

	if (character->npc_type == ENF::Bank)
	{
		int cost = static_cast<int>(character->world->config["BankUpgradeBase"]) + character->bankmax * static_cast<int>(character->world->config["BankUpgradeStep"]);

		if (character->bankmax >= static_cast<int>(character->world->config["MaxBankUpgrades"]))
		{
			return;
		}

		if (character->HasItem(1) < cost)
		{
			return;
		}

		++character->bankmax;
		character->DelItem(1, cost);

		PacketBuilder reply(PACKET_LOCKER, PACKET_BUY, 5);
		reply.AddInt(character->HasItem(1));
		reply.AddChar(character->bankmax);
		character->Send(reply);
	}
}

PACKET_HANDLER_REGISTER(PACKET_LOCKER)
	Register(PACKET_ADD, Locker_Add, Playing);
	Register(PACKET_TAKE, Locker_Take, Playing);
	Register(PACKET_OPEN, Locker_Open, Playing);
	Register(PACKET_BUY, Locker_Buy, Playing);
PACKET_HANDLER_REGISTER_END()

}
