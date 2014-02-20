
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../eodata.hpp"
#include "../map.hpp"
#include "../player.hpp"
#include "../world.hpp"

namespace Handlers
{

void Chest_Add(Character *character, PacketReader &reader)
{
	if (character->trading) return;

	int x = reader.GetChar();
	int y = reader.GetChar();
	int id = reader.GetShort();
	int amount = reader.GetThree();

	if (character->world->eif->Get(id).special == EIF::Lore)
	{
		return;
	}

	if (util::path_length(character->x, character->y, x, y) <= 1)
	{
		if (character->map->GetSpec(x, y) == Map_Tile::Chest)
		{
			UTIL_FOREACH(character->map->chests, chest)
			{
				if (chest->x == x && chest->y == y)
				{
					amount = std::min(amount, int(character->world->config["MaxChest"]) - chest->HasItem(id));

					if (character->HasItem(id) >= amount && chest->AddItem(id, amount))
					{
						character->DelItem(id, amount);
						chest->Update(character->map, character);

						PacketBuilder reply(PACKET_CHEST, PACKET_REPLY, 8 + chest->items.size() * 5);
						reply.AddShort(id);
						reply.AddInt(character->HasItem(id));
						reply.AddChar(character->weight);
						reply.AddChar(character->maxweight);

						UTIL_CIFOREACH(chest->items, item)
						{
							if (item->id != 0)
							{
								reply.AddShort(item->id);
								reply.AddThree(item->amount);
							}
						}

						character->Send(reply);
					}

					break;
				}
			}
		}
	}
}

// Taking an item from a chest
void Chest_Take(Character *character, PacketReader &reader)
{
	int x = reader.GetChar();
	int y = reader.GetChar();
	int id = reader.GetShort();

	if (util::path_length(character->x, character->y, x, y) <= 1)
	{
		if (character->map->GetSpec(x, y) == Map_Tile::Chest)
		{
			UTIL_FOREACH(character->map->chests, chest)
			{
				if (chest->x == x && chest->y == y)
				{
					int amount = chest->HasItem(id);
					int taken = character->CanHoldItem(id, amount);

					if (taken > 0)
					{
						chest->DelSomeItem(id, taken);
						character->AddItem(id, taken);

						PacketBuilder reply(PACKET_CHEST, PACKET_GET, 7 + (chest->items.size() + 1) * 5);
						reply.AddShort(id);
						reply.AddThree(taken);
						reply.AddChar(character->weight);
						reply.AddChar(character->maxweight);

						UTIL_CIFOREACH(chest->items, item)
						{
							if (item->id != 0)
							{
								reply.AddShort(item->id);
								reply.AddThree(item->amount);
							}
						}

						character->Send(reply);

						chest->Update(character->map, character);
						break;
					}
				}
			}
		}
	}
}

// Opening a chest
void Chest_Open(Character *character, PacketReader &reader)
{
	int x = reader.GetChar();
	int y = reader.GetChar();

	if (util::path_length(character->x, character->y, x, y) <= 1)
	{
		if (character->map->GetSpec(x, y) == Map_Tile::Chest)
		{
			PacketBuilder reply(PACKET_CHEST, PACKET_OPEN, 2);
			reply.AddChar(x);
			reply.AddChar(y);

			UTIL_FOREACH(character->map->chests, chest)
			{
				if (chest->x == x && chest->y == y)
				{
					reply.ReserveMore(chest->items.size() * 5);

					UTIL_CIFOREACH(chest->items, item)
					{
						if (item->id != 0)
						{
							reply.AddShort(item->id);
							reply.AddThree(item->amount);
						}
					}

					character->Send(reply);
					break;
				}
			}
		}
	}
}

PACKET_HANDLER_REGISTER(PACKET_CHEST)
	Register(PACKET_ADD, Chest_Add, Playing);
	Register(PACKET_TAKE, Chest_Take, Playing);
	Register(PACKET_OPEN, Chest_Open, Playing);
PACKET_HANDLER_REGISTER_END(PACKET_CHEST)

}
