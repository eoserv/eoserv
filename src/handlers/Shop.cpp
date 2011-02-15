
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

#include "util.hpp"

#include "character.hpp"
#include "map.hpp"
#include "npc.hpp"
#include "player.hpp"

CLIENT_F_FUNC(Shop)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_CREATE: // Crafting an item
		{
			if (this->state < EOClient::Playing) return false;

			short item = reader.GetShort();
			/*int shopid = reader.GetInt();*/

			if (this->player->character->npc_type == ENF::Shop)
			{
				UTIL_FOREACH(this->player->character->npc->shop_craft, checkitem)
				{
					if (checkitem->id == item)
					{
						bool hasitems = true;

						UTIL_FOREACH(checkitem->ingredients, ingredient)
						{
							if (this->player->character->HasItem(ingredient->id) < ingredient->amount)
							{
								hasitems = false;
							}
						}

						if (hasitems)
						{
							reply.SetID(PACKET_SHOP, PACKET_CREATE);
							reply.AddShort(item);
							reply.AddChar(this->player->character->weight);
							reply.AddChar(this->player->character->maxweight);
							UTIL_FOREACH(checkitem->ingredients, ingredient)
							{
								this->player->character->DelItem(ingredient->id, ingredient->amount);
								reply.AddShort(ingredient->id);
								reply.AddInt(this->player->character->HasItem(ingredient->id));
							}
							this->player->character->AddItem(checkitem->id, 1);
							CLIENT_SEND(reply);
						}
					}
				}
			}
		}
		break;

		case PACKET_BUY: // Purchasing an item from a store
		{
			if (this->state < EOClient::Playing) return true;

			short item = reader.GetShort();
			int amount = reader.GetInt();
			/*int shopid = reader.GetInt();*/

			if (amount <= 0 || amount > static_cast<int>(this->server()->world->config["MaxShopBuy"])) return false;

			if (this->player->character->npc_type == ENF::Shop)
			{
				UTIL_FOREACH(this->player->character->npc->shop_trade, checkitem)
				{
					int cost = amount * checkitem->buy;

					if (cost <= 0) return true;

					if (checkitem->id == item && checkitem->buy != 0 && this->player->character->HasItem(1) >= cost)
					{
						this->player->character->DelItem(1, cost);
						this->player->character->AddItem(item, amount);

						reply.SetID(PACKET_SHOP, PACKET_BUY);
						reply.AddInt(this->player->character->HasItem(1));
						reply.AddShort(item);
						reply.AddInt(amount);
						reply.AddChar(this->player->character->weight);
						reply.AddChar(this->player->character->maxweight);
						CLIENT_SEND(reply);
						break;
					}
				}
			}
		}
		break;

		case PACKET_SELL: // Selling an item to a store
		{
			if (this->state < EOClient::Playing) return false;

			short item = reader.GetShort();
			int amount = reader.GetInt();
			/*int shopid = reader.GetInt();*/

			if (amount <= 0) return true;

			if (this->player->character->npc_type == ENF::Shop)
			{
				UTIL_FOREACH(this->player->character->npc->shop_trade, checkitem)
				{
					if (checkitem->id == item && checkitem->sell != 0 && this->player->character->HasItem(item) >= amount)
					{
						this->player->character->DelItem(item, amount);
						this->player->character->AddItem(1, amount * checkitem->sell);

						reply.SetID(PACKET_SHOP, PACKET_SELL);
						reply.AddInt(this->player->character->HasItem(item));
						reply.AddShort(item);
						reply.AddInt(this->player->character->HasItem(1));
						reply.AddChar(this->player->character->weight);
						reply.AddChar(this->player->character->maxweight);
						CLIENT_SEND(reply);
						break;
					}
				}

			}
		}
		break;

		case PACKET_OPEN: // Talking to a store NPC
		{
			if (this->state < EOClient::Playing) return false;
			CLIENT_QUEUE_ACTION(0.0)

			short id = reader.GetShort();

			UTIL_FOREACH(this->player->character->map->npcs, npc)
			{
				if (npc->index == id && (npc->shop_trade.size() > 0 || npc->shop_craft.size() > 0))
				{
					this->player->character->npc = npc;
					this->player->character->npc_type = ENF::Shop;

					reply.SetID(PACKET_SHOP, PACKET_OPEN);
					reply.AddShort(npc->id);
					reply.AddBreakString(npc->shop_name.c_str());

					UTIL_FOREACH(npc->shop_trade, item)
					{
						reply.AddShort(item->id);
						reply.AddThree(item->buy);
						reply.AddThree(item->sell);
						reply.AddChar(static_cast<int>(this->server()->world->config["MaxShopBuy"]));
					}
					reply.AddByte(255);

					UTIL_FOREACH(npc->shop_craft, item)
					{
						std::size_t i = 0;

						reply.AddShort(item->id);

						for (; i < item->ingredients.size(); ++i)
						{
							reply.AddShort(item->ingredients[i]->id);
							reply.AddChar(item->ingredients[i]->amount);
						}

						for (; i < 4; ++i)
						{
							reply.AddShort(0);
							reply.AddChar(0);
						}
					}
					reply.AddByte(255);

					CLIENT_SEND(reply);

					break;
				}
			}
		}
		break;

		default:
			return false;
	}

	return true;
}
