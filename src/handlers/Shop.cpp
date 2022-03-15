/* handlers/Shop.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../config.hpp"
#include "../eodata.hpp"
#include "../map.hpp"
#include "../npc.hpp"
#include "../npc_data.hpp"
#include "../packet.hpp"
#include "../world.hpp"

#include "../util.hpp"

#include <cstddef>
#include <cstdint>

namespace Handlers
{

// Crafting an item
void Shop_Create(Character *character, PacketReader &reader)
{
	if (character->trading) return;

	short item = reader.GetShort();
	/*int shopid = reader.GetInt();*/

	if (character->npc_type == ENF::Shop)
	{
		UTIL_FOREACH_CREF(character->npc->Data().shop_craft, checkitem)
		{
			if (checkitem->id == item)
			{
				bool hasitems = true;

				UTIL_FOREACH(checkitem->ingredients, ingredient)
				{
					if (character->HasItem(ingredient.id) < ingredient.amount)
					{
						hasitems = false;
					}
				}

				if (hasitems)
				{
					PacketBuilder reply(PACKET_SHOP, PACKET_CREATE, 4 + checkitem->ingredients.size() * 6);
					reply.AddShort(item);
					reply.AddChar(character->weight);
					reply.AddChar(character->maxweight);
					UTIL_FOREACH(checkitem->ingredients, ingredient)
					{
						character->DelItem(ingredient.id, ingredient.amount);
						reply.AddShort(ingredient.id);
						reply.AddInt(character->HasItem(ingredient.id));
					}
					character->AddItem(checkitem->id, 1);
					character->Send(reply);
				}
			}
		}
	}
}

// Purchasing an item from a store
void Shop_Buy(Character *character, PacketReader &reader)
{
	if (character->trading) return;

	short item = reader.GetShort();
	int amount = reader.GetInt();
	/*int shopid = reader.GetInt();*/

	if (amount <= 0 || amount > static_cast<int>(character->world->config["MaxShopBuy"])) return;
	if (character->weight >= character->maxweight) return;

	// TODO: Limit number of items bought to under weight

	if (character->npc_type == ENF::Shop)
	{
		UTIL_FOREACH_CREF(character->npc->Data().shop_trade, checkitem)
		{
			if (checkitem->id == item && checkitem->buy != 0)
			{
				std::int_least64_t cost64 = std::int_least64_t(amount) * std::int_least64_t(checkitem->buy);

				if (cost64 < 0 || cost64 > int(character->world->config["MaxItem"]))
					break;

				int cost = int(cost64);

				if (character->HasItem(1) < cost)
					break;

				character->DelItem(1, cost);
				character->AddItem(item, amount);

				PacketBuilder reply(PACKET_SHOP, PACKET_BUY, 12);
				reply.AddInt(character->HasItem(1));
				reply.AddShort(item);
				reply.AddInt(amount);
				reply.AddChar(character->weight);
				reply.AddChar(character->maxweight);
				character->Send(reply);

				break;
			}
		}
	}
}

// Selling an item to a store
void Shop_Sell(Character *character, PacketReader &reader)
{
	if (character->trading) return;

	short item = reader.GetShort();
	int amount = reader.GetInt();
	/*int shopid = reader.GetInt();*/

	if (amount <= 0 || amount > static_cast<int>(character->world->config["MaxItem"])) return;

	if (character->npc_type == ENF::Shop)
	{
		UTIL_FOREACH_CREF(character->npc->Data().shop_trade, checkitem)
		{
			if (checkitem->id == item && checkitem->sell != 0)
			{
				std::int_least64_t owed64 = std::int_least64_t(amount) * std::int_least64_t(checkitem->sell);

				if (owed64 < 0 || owed64 > (int(character->world->config["MaxItem"]) - character->HasItem(1)))
					break;

				int owed = int(owed64);

				if (character->HasItem(item) < amount)
					break;

				character->DelItem(item, amount);
				character->AddItem(1, owed);

				PacketBuilder reply(PACKET_SHOP, PACKET_SELL, 12);
				reply.AddInt(character->HasItem(item));
				reply.AddShort(item);
				reply.AddInt(character->HasItem(1));
				reply.AddChar(character->weight);
				reply.AddChar(character->maxweight);
				character->Send(reply);

				break;
			}
		}

	}
}

// Talking to a store NPC
void Shop_Open(Character *character, PacketReader &reader)
{
	short id = reader.GetShort();

	UTIL_FOREACH(character->map->npcs, npc)
	{
		if (npc->index == id && (npc->Data().shop_trade.size() > 0 || npc->Data().shop_craft.size() > 0))
		{
			character->npc = npc;
			character->npc_type = ENF::Shop;

			PacketBuilder reply(PACKET_SHOP, PACKET_OPEN,
				5 + npc->Data().shop_name.length() + npc->Data().shop_trade.size() * 9 + npc->Data().shop_craft.size() * 14);
			reply.AddShort(npc->id);
			reply.AddBreakString(npc->Data().shop_name.c_str());

			UTIL_FOREACH_CREF(npc->Data().shop_trade, item)
			{
				reply.AddShort(item->id);
				reply.AddThree(item->buy);
				reply.AddThree(item->sell);
				reply.AddChar(static_cast<int>(character->world->config["MaxShopBuy"]));
			}
			reply.AddByte(255);

			UTIL_FOREACH_CREF(npc->Data().shop_craft, item)
			{
				std::size_t i = 0;

				reply.AddShort(item->id);

				for (; i < item->ingredients.size(); ++i)
				{
					reply.AddShort(item->ingredients[i].id);
					reply.AddChar(item->ingredients[i].amount);
				}

				for (; i < 4; ++i)
				{
					reply.AddShort(0);
					reply.AddChar(0);
				}
			}
			reply.AddByte(255);

			character->Send(reply);

			break;
		}
	}
}

PACKET_HANDLER_REGISTER(PACKET_SHOP)
	Register(PACKET_CREATE, Shop_Create, Playing);
	Register(PACKET_BUY, Shop_Buy, Playing);
	Register(PACKET_SELL, Shop_Sell, Playing);
	Register(PACKET_OPEN, Shop_Open, Playing);
PACKET_HANDLER_REGISTER_END(PACKET_SHOP)

}
