
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

#include "eodata.hpp"
#include "guild.hpp"
#include "map.hpp"
#include "npc.hpp"

CLIENT_F_FUNC(Welcome)
{
	PacketBuilder reply;

	unsigned int id;

	switch (action)
	{
		case PACKET_REQUEST: // Selected a character
		{
			if (this->state != EOClient::LoggedIn) return false;

			id = reader.GetInt(); // Character ID

			reply.SetID(PACKET_WELCOME, PACKET_REPLY);

			this->player->character = 0;

			UTIL_PTR_VECTOR_FOREACH(this->player->characters, Character, character)
			{
				if (character->id == id)
				{
					this->player->character = *character;
					character->AddRef();
					break;
				}
			}

			if (!this->player->character)
			{
#ifdef DEBUG
				Console::Wrn("Selected character not found");
#endif
				return false;
			}

			this->player->character->online = true;

			this->player->character->CalculateStats();

			reply.AddShort(1); // REPLY_WELCOME sub-id
			reply.AddShort(this->player->id);
			reply.AddInt(this->player->character->id);
			reply.AddShort(this->player->character->mapid); // Map ID

			if (this->server->world->config["GlobalPK"] && !this->server->world->PKExcept(this->player->character->mapid))
			{
				reply.AddByte(0xFF);
				reply.AddByte(0x01);
			}
			else
			{
				reply.AddByte(this->server->world->GetMap(this->player->character->mapid)->rid[0]);
				reply.AddByte(this->server->world->GetMap(this->player->character->mapid)->rid[1]);
			}

			reply.AddByte(this->server->world->GetMap(this->player->character->mapid)->rid[2]);
			reply.AddByte(this->server->world->GetMap(this->player->character->mapid)->rid[3]);
			reply.AddThree(this->server->world->GetMap(this->player->character->mapid)->filesize);
			reply.AddByte(this->server->world->eif->rid[0]);
			reply.AddByte(this->server->world->eif->rid[1]);
			reply.AddByte(this->server->world->eif->rid[2]);
			reply.AddByte(this->server->world->eif->rid[3]);
			reply.AddByte(this->server->world->eif->len[0]);
			reply.AddByte(this->server->world->eif->len[1]);
			reply.AddByte(this->server->world->enf->rid[0]);
			reply.AddByte(this->server->world->enf->rid[1]);
			reply.AddByte(this->server->world->enf->rid[2]);
			reply.AddByte(this->server->world->enf->rid[3]);
			reply.AddByte(this->server->world->enf->len[0]);
			reply.AddByte(this->server->world->enf->len[1]);
			reply.AddByte(this->server->world->esf->rid[0]);
			reply.AddByte(this->server->world->esf->rid[1]);
			reply.AddByte(this->server->world->esf->rid[2]);
			reply.AddByte(this->server->world->esf->rid[3]);
			reply.AddByte(this->server->world->esf->len[0]);
			reply.AddByte(this->server->world->esf->len[1]);
			reply.AddByte(this->server->world->ecf->rid[0]);
			reply.AddByte(this->server->world->ecf->rid[1]);
			reply.AddByte(this->server->world->ecf->rid[2]);
			reply.AddByte(this->server->world->ecf->rid[3]);
			reply.AddByte(this->server->world->ecf->len[0]);
			reply.AddByte(this->server->world->ecf->len[1]);
			reply.AddBreakString(this->player->character->name);
			reply.AddBreakString(this->player->character->title);
			reply.AddBreakString(this->player->character->guild?this->player->character->guild->name:""); // Guild Name
			reply.AddBreakString(this->player->character->guild?this->player->character->guild->name:""); // Guild Rank
			reply.AddChar(this->player->character->clas);
			reply.AddString(this->player->character->PaddedGuildTag());
			// Tell a guardian's client they're a GM so they can use #nowall
			reply.AddChar((this->player->character->admin == ADMIN_GUARDIAN) ? ADMIN_GM : this->player->character->admin);
			reply.AddChar(this->player->character->level);
			reply.AddInt(this->player->character->exp);
			reply.AddInt(this->player->character->usage);
			reply.AddShort(this->player->character->hp);
			reply.AddShort(this->player->character->maxhp);
			reply.AddShort(this->player->character->tp);
			reply.AddShort(this->player->character->maxtp);
			reply.AddShort(this->player->character->maxsp);
			reply.AddShort(this->player->character->statpoints);
			reply.AddShort(this->player->character->skillpoints);
			reply.AddShort(this->player->character->karma);
			reply.AddShort(this->player->character->mindam);
			reply.AddShort(this->player->character->maxdam);
			reply.AddShort(this->player->character->accuracy);
			reply.AddShort(this->player->character->evade);
			reply.AddShort(this->player->character->armor);
			if (this->version < 28)
			{
				reply.AddChar(this->player->character->str);
				reply.AddChar(this->player->character->wis);
				reply.AddChar(this->player->character->intl);
				reply.AddChar(this->player->character->agi);
				reply.AddChar(this->player->character->con);
				reply.AddChar(this->player->character->cha);
			}
			else
			{
				reply.AddShort(this->player->character->str);
				reply.AddShort(this->player->character->wis);
				reply.AddShort(this->player->character->intl);
				reply.AddShort(this->player->character->agi);
				reply.AddShort(this->player->character->con);
				reply.AddShort(this->player->character->cha);
			}

			CLIENT_SEND(reply);
		}
		break;

		case PACKET_MSG: // Welcome message after you login.
		{
			if (this->state != EOClient::LoggedIn) return false;

			reader.GetThree(); // ??
			id = reader.GetInt(); // Character ID

			if (!this->player->character->world->GetMap(this->player->character->mapid)->exists)
			{
				if (this->player->character->world->GetMap(this->player->character->spawnmap)->exists)
				{
					Console::Wrn("Player logged in to non-existent map (%s, map %i) - Position reset", this->player->character->name.c_str(), this->player->character->mapid);
					this->player->character->Warp(this->player->character->spawnmap, this->player->character->spawnx, this->player->character->spawny);
				}
				else
				{
					Console::Wrn("Player logged in to non-existent map (%s, map %i) - Disconnected", this->player->character->name.c_str(), this->player->character->mapid);
					this->Close();
					return true;
				}
			}

			this->server->world->Login(this->player->character);

			this->state = EOClient::Playing;

			reply.SetID(PACKET_WELCOME, PACKET_REPLY);

			reply.AddShort(2); // REPLY_WELCOME sub-id
			// MotDs
			reply.AddByte(255);

			char newsbuf[4096] = "";
			std::FILE *newsfh = std::fopen(static_cast<std::string>(this->server->world->config["NewsFile"]).c_str(), "rt");
			bool newseof = (newsfh == 0);

			if (newseof)
			{
				Console::Wrn("Could not load news file '%s'", static_cast<std::string>(this->server->world->config["NewsFile"]).c_str());
			}

			for (int i = 0; i < 9; ++i)
			{
				if (newsfh)
				{
					std::fgets(newsbuf, 4096, newsfh);
				}

				if (!newseof)
				{
					reply.AddBreakString(util::trim(newsbuf));
				}
				else
				{
					reply.AddByte(255);
				}

				if (newsfh && std::feof(newsfh))
				{
					newseof = true;
				}

			}

			if (newsfh)
			{
				std::fclose(newsfh);
			}
			// ??
			reply.AddChar(this->player->character->weight); // Weight
			reply.AddChar(this->player->character->maxweight); // Max Weight
			UTIL_PTR_LIST_FOREACH(this->player->character->inventory, Character_Item, item)
			{
				reply.AddShort(item->id);
				reply.AddInt(item->amount);
			}
			reply.AddByte(255);
			// foreach spell {
			//reply.AddShort(1); // Spell ID
			//reply.AddShort(100); // Spell Level
			//reply.AddShort(2); // Spell ID
			//reply.AddShort(100); // Spell Level
			//reply.AddShort(18); // Spell ID
			//reply.AddShort(100); // Spell Level
			// }
			reply.AddByte(255);
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

			reply.AddChar(updatecharacters.size()); // Number of players
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
				reply.AddChar(0); // visible
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

		case PACKET_AGREE: // Client wants a file
		{
			if (this->state != EOClient::LoggedIn) return false;

			std::string content;
			char mapbuf[6] = {0};
			std::sprintf(mapbuf, "%05i", std::abs(this->player->character->mapid));
			std::string filename = this->server->world->config["MapDir"];
			std::FILE *fh;
			InitReply replycode = INIT_FILE_MAP;
			char fileid = 0;

			filename += mapbuf;
			filename += ".emf";

			int file = reader.GetChar();

			switch (file)
			{
				case FILE_MAP: break; // Map file is pre-loaded in to the variable
				case FILE_ITEM: filename = static_cast<std::string>(this->server->world->config["EIF"]); replycode = INIT_FILE_EIF; fileid = 1; break;
				case FILE_NPC: filename = static_cast<std::string>(this->server->world->config["ENF"]); replycode = INIT_FILE_ENF; fileid = 1; break;
				case FILE_SPELL: filename = static_cast<std::string>(this->server->world->config["ESF"]); replycode = INIT_FILE_ESF; fileid = 1; break;
				case FILE_CLASS: filename = static_cast<std::string>(this->server->world->config["ECF"]); replycode = INIT_FILE_ECF; fileid = 1; break;
				default: return false;
			}

			if (file == FILE_MAP && !this->player->character->world->GetMap(this->player->character->mapid)->exists)
			{
				reply.SetID(0);
				reply.AddChar(replycode);
				if (fileid != 0)
				{
					reply.AddChar(fileid);
				}
				reply.AddString(content);
				CLIENT_SENDRAW(reply);
				return true;
			}

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

				if (file == FILE_MAP && this->server->world->config["GlobalPK"] && !this->server->world->PKExcept(this->player->character->mapid))
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
			reply.AddChar(replycode);
			if (fileid != 0)
			{
				reply.AddChar(fileid);
			}
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
