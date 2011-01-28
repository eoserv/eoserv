
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

#include "console.hpp"
#include "eodata.hpp"
#include "map.hpp"
#include "npc.hpp"

CLIENT_F_FUNC(Warp)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_ACCEPT: // Player accepting a warp request from the server
		{
			if (this->state < EOClient::PlayingModal) return false;

			int map = reader.GetShort();

			int anim = this->player->character->warp_anim;

			if (anim != WARP_ANIMATION_INVALID && this->player->character->mapid == map)
			{
				this->player->character->warp_anim = WARP_ANIMATION_INVALID;
			}
			else
			{
				return true;
			}

			PtrVector<Character> updatecharacters;
			PtrVector<NPC> updatenpcs;
			PtrVector<Map_Item> updateitems;

			UTIL_PTR_LIST_FOREACH(this->player->character->map->characters, Character, character)
			{
				if (this->player->character->InRange(*character))
				{
					updatecharacters.push_back(*character);
				}
			}

			UTIL_PTR_VECTOR_FOREACH(this->player->character->map->npcs, NPC, npc)
			{
				if (this->player->character->InRange(*npc))
				{
					updatenpcs.push_back(*npc);
				}
			}

			UTIL_PTR_LIST_FOREACH(this->player->character->map->items, Map_Item, item)
			{
				if (this->player->character->InRange(*item))
				{
					updateitems.push_back(*item);
				}
			}

			reply.SetID(PACKET_WARP, PACKET_AGREE);
			reply.AddChar(2); // ?
			reply.AddShort(map);
			reply.AddChar(anim);
			reply.AddChar(updatecharacters.size());
			reply.AddByte(255);
			UTIL_PTR_VECTOR_FOREACH(updatecharacters, Character, character)
			{
				reply.AddBreakString(character->name);
				reply.AddShort(character->player->id);
				reply.AddShort(character->mapid);
				reply.AddShort(character->x);
				reply.AddShort(character->y);
				reply.AddChar(character->direction);
				reply.AddChar(6); // ?
				reply.AddString(character->PaddedGuildTag());
				reply.AddChar(character->level);
				reply.AddChar(character->gender);
				reply.AddChar(character->hairstyle);
				reply.AddChar(character->haircolor);
				reply.AddChar(character->race);
				reply.AddShort(character->maxhp);
				reply.AddShort(character->hp);
				reply.AddShort(character->maxtp);
				reply.AddShort(character->tp);
				// equipment
				reply.AddShort(this->server->world->eif->Get(character->paperdoll[Character::Boots])->dollgraphic);
				reply.AddShort(0); // ??
				reply.AddShort(0); // ??
				reply.AddShort(0); // ??
				reply.AddShort(this->server->world->eif->Get(character->paperdoll[Character::Armor])->dollgraphic);
				reply.AddShort(0); // ??
				reply.AddShort(this->server->world->eif->Get(character->paperdoll[Character::Hat])->dollgraphic);
				reply.AddShort(this->server->world->eif->Get(character->paperdoll[Character::Shield])->dollgraphic);
				reply.AddShort(this->server->world->eif->Get(character->paperdoll[Character::Weapon])->dollgraphic);
				reply.AddChar(character->sitting);
				reply.AddChar(character->hidden);
				reply.AddByte(255);
			}
			UTIL_PTR_VECTOR_FOREACH(updatenpcs, NPC, npc)
			{
				if (npc->alive)
				{
					reply.AddChar(npc->index);
					reply.AddShort(npc->Data()->id);
					reply.AddChar(npc->x);
					reply.AddChar(npc->y);
					reply.AddChar(npc->direction);
				}
			}
			reply.AddByte(255);
			UTIL_PTR_VECTOR_FOREACH(updateitems, Map_Item, item)
			{
				reply.AddShort(item->uid);
				reply.AddShort(item->id);
				reply.AddChar(item->x);
				reply.AddChar(item->y);
				reply.AddThree(item->amount);
			}
			CLIENT_SEND(reply);

		}
		break;

		case PACKET_TAKE: // Player needs a copy of the map they're being warped to
		{
			if (this->state < EOClient::PlayingModal) return false;

			char mapbuf[6] = {0};
			std::sprintf(mapbuf, "%05i", std::abs(this->player->character->mapid));
			std::string filename = this->server->world->config["MapDir"];
			std::string content;
			std::FILE *fh;

			filename += mapbuf;
			filename += ".emf";

			fh = std::fopen(filename.c_str(), "rb");

			if (!fh)
			{
				Console::Err("Could not load file: %s", filename.c_str());
				return false;
			}

			int p = 0;
			do {
				char buf[4096];
				int len = std::fread(buf, sizeof(char), 4096, fh);

				if (this->server->world->config["GlobalPK"] && !this->server->world->PKExcept(this->player->character->mapid))
				{
					if (p + len >= 0x04 && 0x03 - p > 0) buf[0x03 - p] = 0xFF;
					if (p + len >= 0x05 && 0x04 - p > 0) buf[0x04 - p] = 0x01;
					if (p + len >= 0x20 && 0x1F - p > 0) buf[0x1F - p] = 0x04;
				}

				p += len;
				content.append(buf, len);
			} while (!std::feof(fh));

			std::fclose(fh);

			reply.SetID(0);
			reply.AddChar(INIT_BANNED); // wtf? When in Rome...
			reply.AddString(content);
			CLIENT_SENDRAW(reply);

			if (this->server->world->config["ProtectMaps"])
			{
				reply.Reset();
				reply.AddChar(INIT_BANNED);
				CLIENT_SENDRAW(reply);
			}
		}
		break;

		default:
			return false;
	}

	return true;
}
