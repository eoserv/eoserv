
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../util.hpp"

#include "../character.hpp"
#include "../eodata.hpp"
#include "../map.hpp"
#include "../npc.hpp"
#include "../player.hpp"

namespace Handlers
{

// Talked to a barber NPC
void Barber_Open(Character *character, PacketReader &reader)
{
	short id = reader.GetShort();

	UTIL_FOREACH(character->map->npcs, npc)
	{
		if (npc->index == id && npc->Data().type == ENF::Barber)
		{
			character->npc = npc;
			character->npc_type = ENF::Barber;

			PacketBuilder reply(PACKET_BARBER, PACKET_OPEN, 4);
			reply.AddInt(0); // Session token

			character->Send(reply);

			break;
		}
	}
}

// Purchased a hair style
void Barber_Buy(Character *character, PacketReader &reader)
{
	if (character->trading) return;

	unsigned char style = reader.GetChar();
	unsigned char color = reader.GetChar();

	if (style > static_cast<int>(character->world->config["MaxHairStyle"])
	 || color > static_cast<int>(character->world->config["MaxHairColor"]))
	{
		return;
	}

	int price = character->world->config["BarberBase"];
	price += std::max(1, int(character->level)) * static_cast<int>(character->world->config["BarberStep"]);

	if (character->HasItem(1) < price)
	{
		return;
	}

	if (character->npc_type == ENF::Barber)
	{
		character->DelItem(1, price);

		character->hairstyle = style;
		character->haircolor = color;

		PacketBuilder reply(PACKET_BARBER, PACKET_AGREE, 10);
		reply.AddInt(character->HasItem(1));
		reply.AddShort(character->player->id);
		reply.AddChar(SLOT_HAIR);
		reply.AddChar(0); // subloc
		reply.AddChar(style);
		reply.AddChar(color);

		PacketBuilder builder(PACKET_AVATAR, PACKET_AGREE, 6);
		builder.AddShort(character->player->id);
		builder.AddChar(SLOT_HAIR);
		builder.AddChar(0); // subloc
		builder.AddChar(style);
		builder.AddChar(color);

		UTIL_FOREACH(character->map->characters, updatecharacter)
		{
			if (updatecharacter != character && character->InRange(updatecharacter))
			{
				updatecharacter->Send(builder);
			}
		}

		character->Send(reply);
	}
}

PACKET_HANDLER_REGISTER(PACKET_BARBER)
	Register(PACKET_OPEN, Barber_Open, Playing);
	Register(PACKET_BUY, Barber_Buy, Playing);
PACKET_HANDLER_REGISTER_END(PACKET_BARBER)

}
