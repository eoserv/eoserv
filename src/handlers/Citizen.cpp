
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

#include "map.hpp"
#include "npc.hpp"

CLIENT_F_FUNC(Citizen)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Sleeping at an inn
		{
			// TODO: Sleeping at inns
		}
		break;

		case PACKET_REPLY: // Player subscribing to a town
		{
			if (this->state < EOClient::Playing) return false;

			std::string answers[3];

			/*int session = */reader.GetShort();
			reader.GetByte();
			/*short npcid = */reader.GetShort();
			reader.GetByte();
			answers[0] = reader.GetBreakString();
			answers[1] = reader.GetBreakString();
			answers[2] = reader.GetEndString();

			if (this->player->character->npc_type == ENF::Inn)
			{
				int questions_wrong = 0;

				for (int i = 0; i < 3; ++i)
				{
					if (util::lowercase(answers[i]) != util::lowercase(this->player->character->npc->citizenship->answers[i]))
					{
						++questions_wrong;
					}
				}

				if (questions_wrong == 0)
				{
					this->player->character->home = this->player->character->npc->citizenship->home;
				}

				reply.SetID(PACKET_CITIZEN, PACKET_REPLY);
				reply.AddChar(questions_wrong);

				CLIENT_SEND(reply);
			}
		}
		break;

		case PACKET_REMOVE: // Player giving up citizenship of a town
		{
			if (this->state < EOClient::Playing) return false;

			/*short npcid = reader.GetShort();*/

			if (this->player->character->npc_type == ENF::Inn)
			{
				reply.SetID(PACKET_CITIZEN, PACKET_REMOVE);

				if (this->player->character->home == this->player->character->npc->citizenship->home)
				{
					this->player->character->home = "";
					reply.AddChar(UNSUBSCRIBE_UNSUBSCRIBED);
				}
				else
				{
					reply.AddChar(UNSUBSCRIBE_NOT_CITIZEN);
				}

				CLIENT_SEND(reply);
			}
		}
		break;

		case PACKET_OPEN: // Talked to a citizenship NPC
		{
			if (this->state < EOClient::Playing) return false;
			CLIENT_QUEUE_ACTION(0.0)

			short id = reader.GetShort();

			UTIL_PTR_VECTOR_FOREACH(this->player->character->map->npcs, NPC, npc)
			{
				if (npc->index == id && npc->Data()->type == ENF::Inn && npc->citizenship)
				{
					this->player->character->npc = *npc;
					this->player->character->npc_type = ENF::Inn;

					reply.SetID(PACKET_CITIZEN, PACKET_OPEN);
					reply.AddThree(1); // ?
					reply.AddChar(0); // ?
					reply.AddShort(0); // session
					reply.AddByte(255);
					reply.AddBreakString(npc->citizenship->questions[0]);
					reply.AddBreakString(npc->citizenship->questions[1]);
					reply.AddString(npc->citizenship->questions[2]);

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
