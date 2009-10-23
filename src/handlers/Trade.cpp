
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

#include "eodata.hpp"
#include "map.hpp"

CLIENT_F_FUNC(Trade)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Requesting a trade with another player
		{
			if (this->state < EOClient::Playing) return false;

			int something = reader.GetChar(); // ?
			int victimid = reader.GetShort();

			Character *victim = this->player->character->map->GetCharacterPID(victimid);

			if (victim == this->player->character)
			{
				return true;
			}

			if (victim)
			{
				PacketBuilder builder(PACKET_TRADE, PACKET_REQUEST);
				builder.AddChar(something);
				builder.AddShort(this->player->character->id);
				builder.AddString(this->player->character->name);
				victim->player->client->SendBuilder(builder);

				this->player->character->trade_partner = victim;
			}
		}
		break;

		case PACKET_ACCEPT: // Accepting a trade request
		{
			if (this->state < EOClient::Playing) return false;

			/*int accept =*/ reader.GetChar();
			int victimid = reader.GetShort();

			Character *victim = this->player->character->map->GetCharacterCID(victimid);

			if (victim && victim->mapid == this->player->character->mapid && victim->trade_partner == this->player->character && (victim->player->client->state > EOClient::PlayingModal))
			{
				PacketBuilder builder(PACKET_TRADE, PACKET_OPEN);
				builder.AddShort(victim->id);
				builder.AddBreakString(victim->name);
				builder.AddShort(this->player->character->id);
				builder.AddBreakString(this->player->character->name);
				CLIENT_SEND(builder);
				victim->player->client->SendBuilder(builder);

				this->player->character->trade_partner = victim;
				this->state = EOClient::PlayingModal;
				this->player->character->trading = true;
				this->player->character->trade_agree = false;
				victim->player->client->state = EOClient::PlayingModal;
				victim->trading = true;
				victim->trade_agree = false;
			}
		}
		break;

		case PACKET_REMOVE: // Remove an item from the trade screen
		{
			if (this->state < EOClient::PlayingModal || !this->player->character->trading) return false;

			int itemid = reader.GetShort();

			if (this->player->character->trade_inventory.size() > 5)
			{
				return true;
			}

			if (this->player->character->DelTradeItem(itemid))
			{
				this->player->character->trade_agree = false;
				this->player->character->trade_partner->trade_agree = false;
				PacketBuilder builder(PACKET_TRADE, PACKET_REPLY);

				builder.AddShort(this->player->character->player->id);
				UTIL_LIST_FOREACH_ALL(this->player->character->trade_inventory, Character_Item, item)
				{
					builder.AddShort(item.id);
					builder.AddInt(item.amount);
				}
				builder.AddByte(255);

				builder.AddShort(this->player->character->trade_partner->player->id);
				UTIL_LIST_FOREACH_ALL(this->player->character->trade_partner->trade_inventory, Character_Item, item)
				{
					builder.AddShort(item.id);
					builder.AddInt(item.amount);
				}
				builder.AddByte(255);

				CLIENT_SEND(builder);
				this->player->character->trade_partner->player->client->SendBuilder(builder);
			}

		}
		break;

		case PACKET_AGREE: // Mark your (dis)agreeance with the current trade
		{
			if (this->state < EOClient::PlayingModal || !this->player->character->trading) return false;

			int agree = reader.GetChar();

			this->player->character->trade_agree = agree;

			if (agree)
			{
				if (this->player->character->trade_inventory.empty())
				{
					return true;
				}

				if (this->player->character->trade_partner->trade_agree)
				{
					PacketBuilder builder(PACKET_TRADE, PACKET_USE);
					builder.AddShort(this->player->character->trade_partner->player->id);
					UTIL_LIST_FOREACH_ALL(this->player->character->trade_partner->trade_inventory, Character_Item, item)
					{
						builder.AddShort(item.id);
						builder.AddInt(item.amount);
						this->player->character->trade_partner->DelItem(item.id, item.amount);
						this->player->character->AddItem(item.id, item.amount);
					}
					builder.AddByte(255);
					builder.AddShort(this->player->character->player->id);
					UTIL_LIST_FOREACH_ALL(this->player->character->trade_inventory, Character_Item, item)
					{
						builder.AddShort(item.id);
						builder.AddInt(item.amount);
						this->player->character->DelItem(item.id, item.amount);
						this->player->character->trade_partner->AddItem(item.id, item.amount);
					}
					builder.AddByte(255);
					CLIENT_SEND(builder);
					this->player->character->trade_partner->player->client->SendBuilder(builder);

					this->player->character->Emote(EMOTE_TRADE);
					this->player->character->trade_partner->Emote(EMOTE_TRADE);

					this->state = EOClient::Playing;
					this->player->character->trading = false;
					this->player->character->trade_inventory.clear();
					this->player->character->trade_agree = false;

					this->player->character->trade_partner->player->client->state = EOClient::Playing;
					this->player->character->trade_partner->trading = false;
					this->player->character->trade_partner->trade_inventory.clear();
					this->player->character->trade_agree = false;

					this->player->character->trade_partner->trade_partner = 0;
					this->player->character->trade_partner = 0;
				}
				else
				{
					reply.SetID(PACKET_TRADE, PACKET_SPEC);
					reply.AddChar(agree);
					CLIENT_SEND(reply);
					PacketBuilder builder(PACKET_TRADE, PACKET_AGREE);
					builder.AddShort(this->player->character->trade_partner->id);
					builder.AddChar(agree);
					this->player->character->trade_partner->player->client->SendBuilder(builder);
				}
			}
			else
			{
				reply.SetID(PACKET_TRADE, PACKET_SPEC);
				reply.AddChar(agree);
				CLIENT_SEND(reply);
				PacketBuilder builder(PACKET_TRADE, PACKET_AGREE);
				builder.AddShort(this->player->character->trade_partner->id);
				builder.AddChar(agree);
				this->player->character->trade_partner->player->client->SendBuilder(builder);
			}
		}
		break;

		case PACKET_ADD: // Add an item to the trade screen
		{
			if (this->state < EOClient::PlayingModal || !this->player->character->trading) return false;

			int itemid = reader.GetShort();
			int amount = reader.GetInt();

			if (this->server->world->eif->Get(id)->special == EIF::Lore)
			{
				return true;
			}

			bool offered = false;
			UTIL_LIST_FOREACH_ALL(this->player->character->trade_inventory, Character_Item, item)
			{
				if (item.id == itemid)
				{
					offered = true;
					break;
				}
			}

			if (!offered && this->player->character->trade_inventory.size() >= 10)
			{
				return true;
			}

			if (this->player->character->AddTradeItem(itemid, amount))
			{
				this->player->character->trade_agree = false;
				this->player->character->trade_partner->trade_agree = false;
				PacketBuilder builder(PACKET_TRADE, PACKET_REPLY);

				builder.AddShort(this->player->character->player->id);
				UTIL_LIST_FOREACH_ALL(this->player->character->trade_inventory, Character_Item, item)
				{
					builder.AddShort(item.id);
					builder.AddInt(item.amount);
				}
				builder.AddByte(255);

				builder.AddShort(this->player->character->trade_partner->player->id);
				UTIL_LIST_FOREACH_ALL(this->player->character->trade_partner->trade_inventory, Character_Item, item)
				{
					builder.AddShort(item.id);
					builder.AddInt(item.amount);
				}
				builder.AddByte(255);

				CLIENT_SEND(builder);
				this->player->character->trade_partner->player->client->SendBuilder(builder);
			}
		}
		break;

		case PACKET_CLOSE: // Cancel the trade
		{
			if (this->state < EOClient::PlayingModal || !this->player->character->trading) return false;

			/*int something =*/ reader.GetChar();

			PacketBuilder builder(PACKET_TRADE, PACKET_CLOSE);
			builder.AddShort(this->player->character->id);
			this->player->character->trade_partner->player->client->SendBuilder(builder);

			this->state = EOClient::Playing;
			this->player->character->trading = false;
			this->player->character->trade_inventory.clear();
			this->player->character->trade_agree = false;

			this->player->character->trade_partner->player->client->state = EOClient::Playing;
			this->player->character->trade_partner->trading = false;
			this->player->character->trade_partner->trade_inventory.clear();
			this->player->character->trade_agree = false;

			this->player->character->trade_partner->trade_partner = 0;
			this->player->character->trade_partner = 0;
		}
		break;

		default:
			return false;
	}

	return true;
}
