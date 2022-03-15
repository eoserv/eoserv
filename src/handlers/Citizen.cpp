/* handlers/Citizen.cpp
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

#include <string>

namespace Handlers
{

// Sleeping at an inn
void Citizen_Request(Character *character, PacketReader &reader)
{
	// TODO: Sleeping at inns
	(void)character;
	(void)reader;
}

// Player subscribing to a town
void Citizen_Reply(Character *character, PacketReader &reader)
{
	std::string answers[3];

	/*int session = */reader.GetShort();
	reader.GetByte();
	/*short npcid = */reader.GetShort();
	reader.GetByte();
	answers[0] = reader.GetBreakString();
	answers[1] = reader.GetBreakString();
	answers[2] = reader.GetEndString();

	if (character->npc_type == ENF::Inn && (character->home.size() == 0 || character->world->config["CitizenSubscribeAnytime"]))
	{
		int questions_wrong = 0;

		for (int i = 0; i < 3; ++i)
		{
			if (util::lowercase(answers[i]) != util::lowercase(character->npc->Data().citizenship->answers[i]))
			{
				++questions_wrong;
			}
		}

		if (questions_wrong == 0)
		{
			character->home = character->npc->Data().citizenship->home;
		}

		PacketBuilder reply(PACKET_CITIZEN, PACKET_REPLY, 1);
		reply.AddChar(questions_wrong);

		character->Send(reply);
	}
}

// Player giving up citizenship of a town
void Citizen_Remove(Character *character, PacketReader &reader)
{
	(void)reader;
	/*short npcid = reader.GetShort();*/

	if (character->npc_type == ENF::Inn)
	{
		PacketBuilder reply(PACKET_CITIZEN, PACKET_REMOVE, 1);

		if (character->home == character->npc->Data().citizenship->home || character->world->config["CitizenUnsubscribeAnywhere"])
		{
			character->home = "";
			reply.AddChar(UNSUBSCRIBE_UNSUBSCRIBED);
		}
		else
		{
			reply.AddChar(UNSUBSCRIBE_NOT_CITIZEN);
		}

		character->Send(reply);
	}
}

// Talked to a citizenship NPC
void Citizen_Open(Character *character, PacketReader &reader)
{
	short id = reader.GetShort();

	UTIL_FOREACH(character->map->npcs, npc)
	{
		if (npc->index == id && npc->ENF().type == ENF::Inn && npc->Data().citizenship)
		{
			character->npc = npc;
			character->npc_type = ENF::Inn;

			PacketBuilder reply(PACKET_CITIZEN, PACKET_OPEN,
				9 + npc->Data().citizenship->questions[0].length() + npc->Data().citizenship->questions[1].length() + npc->Data().citizenship->questions[2].length());

			Home* home = character->world->GetHome(character);
			int innkeeper_vend = 0;

			if (home)
				innkeeper_vend = home->innkeeper_vend;

			reply.AddThree(npc->ENF().vendor_id);

			if (character->world->config["CitizenSubscribeAnytime"] && innkeeper_vend != npc->ENF().vendor_id)
				reply.AddChar(0);
			else
				reply.AddChar(innkeeper_vend);

			reply.AddShort(0); // session (should match global warp ID)
			reply.AddByte(255);
			reply.AddBreakString(npc->Data().citizenship->questions[0]);
			reply.AddBreakString(npc->Data().citizenship->questions[1]);
			reply.AddString(npc->Data().citizenship->questions[2]);

			character->Send(reply);

			break;
		}
	}
}

PACKET_HANDLER_REGISTER(PACKET_CITIZEN)
	// Register(PACKET_REQUEST, Citizen_Request, Playing);
	Register(PACKET_REPLY, Citizen_Reply, Playing);
	Register(PACKET_REMOVE, Citizen_Remove, Playing);
	Register(PACKET_OPEN, Citizen_Open, Playing);
PACKET_HANDLER_REGISTER_END(PACKET_CITIZEN)

}
