
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "character.hpp"
#include "eodata.hpp"
#include "map.hpp"
#include "player.hpp"
#include "world.hpp"

namespace Handlers
{

void Chest_Add(Character *character, PacketReader &reader)
{
	if (character->trading) return;

	int x = reader.GetChar();
	int y = reader.GetChar();
	int id = reader.GetShort();
	int amount = reader.GetThree();

	if (character->world->eif->Get(id)->special == EIF::Lore)
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
					if (character->HasItem(id) >= amount && chest->AddItem(id, amount))
					{
						character->DelItem(id, amount);
						chest->Update(character->map, character);

						PacketBuilder reply(PACKET_CHEST, PACKET_REPLY);
						reply.AddShort(id);
						reply.AddInt(character->HasItem(id));
						reply.AddChar(character->weight);
						reply.AddChar(character->maxweight);

						UTIL_FOREACH(chest->items, item)
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
					int amount = chest->DelItem(id);

					if (amount)
					{
						character->AddItem(id, amount);
						chest->Update(character->map, character);

						PacketBuilder reply(PACKET_CHEST, PACKET_GET);
						reply.AddShort(id);
						reply.AddThree(amount);
						reply.AddChar(character->weight);
						reply.AddChar(character->maxweight);

						UTIL_FOREACH(chest->items, item)
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
			PacketBuilder reply(PACKET_CHEST, PACKET_OPEN);
			reply.AddChar(x);
			reply.AddChar(y);

			UTIL_FOREACH(character->map->chests, chest)
			{
				if (chest->x == x && chest->y == y)
				{
					UTIL_FOREACH(chest->items, item)
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
PACKET_HANDLER_REGISTER_END()

}
