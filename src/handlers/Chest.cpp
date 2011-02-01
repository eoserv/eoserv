
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

#include "character.hpp"
#include "eodata.hpp"
#include "map.hpp"
#include "player.hpp"
#include "world.hpp"

CLIENT_F_FUNC(Chest)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_ADD: // Placing an item in a chest
		{
			if (this->state < EOClient::Playing) return false;

			int x = reader.GetChar();
			int y = reader.GetChar();
			int id = reader.GetShort();
			int amount = reader.GetThree();

			if (this->server()->world->eif->Get(id)->special == EIF::Lore)
			{
				return true;
			}

			if (util::path_length(this->player->character->x, this->player->character->y, x, y) <= 1)
			{
				if (this->player->character->map->GetSpec(x, y) == Map_Tile::Chest)
				{
					UTIL_PTR_VECTOR_FOREACH(this->player->character->map->chests, Map_Chest, chest)
					{
						if (chest->x == x && chest->y == y)
						{
							if (this->player->character->HasItem(id) >= amount && chest->AddItem(id, amount))
							{
								this->player->character->DelItem(id, amount);
								chest->Update(this->player->character->map, this->player->character);

								reply.SetID(PACKET_CHEST, PACKET_REPLY);
								reply.AddShort(id);
								reply.AddInt(this->player->character->HasItem(id));
								reply.AddChar(this->player->character->weight);
								reply.AddChar(this->player->character->maxweight);

								UTIL_PTR_LIST_FOREACH(chest->items, Map_Chest_Item, item)
								{
									if (item->id != 0)
									{
										reply.AddShort(item->id);
										reply.AddThree(item->amount);
									}
								}

								CLIENT_SEND(reply);
								break;
							}
						}
					}
				}
			}
		}
		break;

		case PACKET_TAKE: // Taking an item from a chest
		{
			if (this->state < EOClient::Playing) return false;

			int x = reader.GetChar();
			int y = reader.GetChar();
			int id = reader.GetShort();

			if (util::path_length(this->player->character->x, this->player->character->y, x, y) <= 1)
			{
				if (this->player->character->map->GetSpec(x, y) == Map_Tile::Chest)
				{
					UTIL_PTR_VECTOR_FOREACH(this->player->character->map->chests, Map_Chest, chest)
					{
						if (chest->x == x && chest->y == y)
						{
							int amount = chest->DelItem(id);

							if (amount)
							{
								this->player->character->AddItem(id, amount);
								chest->Update(this->player->character->map, this->player->character);

								reply.SetID(PACKET_CHEST, PACKET_GET);
								reply.AddShort(id);
								reply.AddThree(amount);
								reply.AddChar(this->player->character->weight);
								reply.AddChar(this->player->character->maxweight);

								UTIL_PTR_LIST_FOREACH(chest->items, Map_Chest_Item, item)
								{
									if (item->id != 0)
									{
										reply.AddShort(item->id);
										reply.AddThree(item->amount);
									}
								}

								CLIENT_SEND(reply);
								break;
							}
						}
					}
				}
			}
		}
		break;

		case PACKET_OPEN: // Opening a chest
		{
			if (this->state < EOClient::Playing) return false;
			CLIENT_QUEUE_ACTION(0.0)

			int x = reader.GetChar();
			int y = reader.GetChar();

			if (util::path_length(this->player->character->x, this->player->character->y, x, y) <= 1)
			{
				if (this->player->character->map->GetSpec(x, y) == Map_Tile::Chest)
				{
					reply.SetID(PACKET_CHEST, PACKET_OPEN);
					reply.AddChar(x);
					reply.AddChar(y);

					UTIL_PTR_VECTOR_FOREACH(this->player->character->map->chests, Map_Chest, chest)
					{
						if (chest->x == x && chest->y == y)
						{
							UTIL_PTR_LIST_FOREACH(chest->items, Map_Chest_Item, item)
							{
								if (item->id != 0)
								{
									reply.AddShort(item->id);
									reply.AddThree(item->amount);
								}
							}

							CLIENT_SEND(reply);
							break;
						}
					}
				}
			}
		}
		break;

		default:
			return false;
	}

	return true;
}
