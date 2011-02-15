
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

#include "util.hpp"

#include "character.hpp"
#include "eodata.hpp"
#include "map.hpp"
#include "npc.hpp"
#include "player.hpp"

CLIENT_F_FUNC(Barber)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_OPEN: // Talked to a barber NPC
		{
			if (this->state < EOClient::Playing) return false;
			CLIENT_QUEUE_ACTION(0.0)

			short id = reader.GetShort();

			UTIL_FOREACH(this->player->character->map->npcs, npc)
			{
				if (npc->index == id && npc->Data()->type == ENF::Barber)
				{
					this->player->character->npc = npc;
					this->player->character->npc_type = ENF::Barber;

					reply.SetID(PACKET_BARBER, PACKET_OPEN);
					reply.AddInt(0); // Session token

					CLIENT_SEND(reply);

					break;
				}
			}
		}
		break;

		case PACKET_BUY: // Purchased a hair style
		{
			if (this->state < EOClient::Playing) return false;
			CLIENT_QUEUE_ACTION(0.0)

			unsigned char style = reader.GetChar();
			unsigned char color = reader.GetChar();

			if (style < 0 || style > static_cast<int>(this->server()->world->config["MaxHairStyle"])
			 || color < 0 || color > static_cast<int>(this->server()->world->config["MaxHairColor"]))
			{
				return false;
			}

			int price = this->server()->world->config["BarberBase"];
			price += std::max(1, int(this->player->character->level)) * static_cast<int>(this->server()->world->config["BarberStep"]);

			if (this->player->character->HasItem(1) < price)
			{
				return false;
			}

			if (this->player->character->npc_type == ENF::Barber)
			{
				this->player->character->DelItem(1, price);

				this->player->character->hairstyle = style;
				this->player->character->haircolor = color;

				reply.SetID(PACKET_BARBER, PACKET_AGREE);
				reply.AddInt(this->player->character->HasItem(1));
				reply.AddShort(this->player->id);
				reply.AddChar(SLOT_HAIR);
				reply.AddChar(0); // subloc
				reply.AddChar(style);
				reply.AddChar(color);

				PacketBuilder builder(PACKET_CLOTHES, PACKET_AGREE);
				builder.AddShort(this->player->id);
				builder.AddChar(SLOT_HAIR);
				builder.AddChar(0); // subloc
				builder.AddChar(style);
				builder.AddChar(color);

				UTIL_FOREACH(this->player->character->map->characters, character)
				{
					if (character != this->player->character && this->player->character->InRange(character))
					{
						character->player->client->SendBuilder(builder);
					}
				}

				CLIENT_SEND(reply);
			}
		}
		break;

		default:
			return false;
	}

	return true;
}
