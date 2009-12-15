
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

#include "eodata.hpp"
#include "map.hpp"
#include "npc.hpp"

CLIENT_F_FUNC(Bank)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_OPEN: // Talked to a banker NPC
		{
			if (this->state < EOClient::Playing) return false;

			short id = reader.GetShort();

			UTIL_PTR_VECTOR_FOREACH(this->player->character->map->npcs, NPC, npc)
			{
				if (npc->index == id && npc->data->type == ENF::Bank)
				{
					this->player->character->bank_npc = *npc;

					reply.SetID(PACKET_BANK, PACKET_OPEN);
					reply.AddInt(this->player->character->goldbank);
					reply.AddThree(0); // Session token
					reply.AddChar(this->player->character->bankmax);

					CLIENT_SEND(reply);

					break;
				}
			}

		}
		break;

		case PACKET_ADD: // Depositing gold
		{
			if (this->state < EOClient::Playing) return false;

			int amount = reader.GetInt();

			if (amount <= 0) return true;

			amount = std::min(amount, this->player->character->HasItem(1));

			if (this->player->character->bank_npc)
			{
				int newgold = this->player->character->goldbank + amount;

				if (newgold < this->player->character->goldbank || newgold > static_cast<int>(this->server->world->config["MaxBankGold"]))
				{
					return true;
				}

				this->player->character->DelItem(1, amount);
				this->player->character->goldbank = newgold;

				reply.SetID(PACKET_BANK, PACKET_REPLY);
				reply.AddInt(this->player->character->HasItem(1));
				reply.AddInt(this->player->character->goldbank);
				CLIENT_SEND(reply);
			}
		}
		break;

		case PACKET_TAKE: // Withdrawing gold
		{
			if (this->state < EOClient::Playing) return false;

			int amount = reader.GetInt();

			if (amount <= 0) return true;

			if (this->player->character->bank_npc)
			{
				int newgold = this->player->character->goldbank - amount;

				if (newgold > this->player->character->goldbank || newgold < 0)
				{
					return true;
				}

				this->player->character->goldbank = newgold;
				this->player->character->AddItem(1, amount);

				reply.SetID(PACKET_BANK, PACKET_REPLY);
				reply.AddInt(this->player->character->HasItem(1));
				reply.AddInt(this->player->character->goldbank);
				CLIENT_SEND(reply);
			}

		}
		break;

		default:
			return false;
	}

	return true;
}
