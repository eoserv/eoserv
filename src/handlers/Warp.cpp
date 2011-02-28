
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include <vector>

#include "character.hpp"
#include "console.hpp"
#include "eodata.hpp"
#include "map.hpp"
#include "npc.hpp"
#include "player.hpp"

namespace Handlers
{

// Player accepting a warp request from the server
void Warp_Accept(Character *character, PacketReader &reader)
{
	int map = reader.GetShort();

	int anim = character->warp_anim;

	if (anim != WARP_ANIMATION_INVALID && character->mapid == map)
	{
		character->warp_anim = WARP_ANIMATION_INVALID;
	}
	else
	{
		return;
	}

	std::vector<Character *> updatecharacters;
	std::vector<NPC *> updatenpcs;
	std::vector<Map_Item *> updateitems;

	UTIL_FOREACH(character->map->characters, character)
	{
		if (character->InRange(character))
		{
			updatecharacters.push_back(character);
		}
	}

	UTIL_FOREACH(character->map->npcs, npc)
	{
		if (character->InRange(npc))
		{
			updatenpcs.push_back(npc);
		}
	}

	UTIL_FOREACH(character->map->items, item)
	{
		if (character->InRange(item))
		{
			updateitems.push_back(item);
		}
	}

	PacketBuilder reply(PACKET_WARP, PACKET_AGREE);
	reply.AddChar(2); // ?
	reply.AddShort(map);
	reply.AddChar(anim);
	reply.AddChar(updatecharacters.size());
	reply.AddByte(255);
	UTIL_FOREACH(updatecharacters, character)
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
		reply.AddShort(character->world->eif->Get(character->paperdoll[Character::Boots])->dollgraphic);
		reply.AddShort(0); // ??
		reply.AddShort(0); // ??
		reply.AddShort(0); // ??
		reply.AddShort(character->world->eif->Get(character->paperdoll[Character::Armor])->dollgraphic);
		reply.AddShort(0); // ??
		reply.AddShort(character->world->eif->Get(character->paperdoll[Character::Hat])->dollgraphic);
		reply.AddShort(character->world->eif->Get(character->paperdoll[Character::Shield])->dollgraphic);
		reply.AddShort(character->world->eif->Get(character->paperdoll[Character::Weapon])->dollgraphic);
		reply.AddChar(character->sitting);
		reply.AddChar(character->hidden);
		reply.AddByte(255);
	}
	UTIL_FOREACH(updatenpcs, npc)
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
	UTIL_FOREACH(updateitems, item)
	{
		reply.AddShort(item->uid);
		reply.AddShort(item->id);
		reply.AddChar(item->x);
		reply.AddChar(item->y);
		reply.AddThree(item->amount);
	}
	character->Send(reply);
}

// Player needs a copy of the map they're being warped to
void Warp_Take(Character *character, PacketReader &reader)
{
	(void)reader;

	char mapbuf[6] = {0};
	std::sprintf(mapbuf, "%05i", int(std::abs(character->mapid)));
	std::string filename = character->world->config["MapDir"];
	std::string content;
	std::FILE *fh;

	filename += mapbuf;
	filename += ".emf";

	fh = std::fopen(filename.c_str(), "rb");

	if (!fh)
	{
		Console::Err("Could not load file: %s", filename.c_str());
		return;
	}

	int p = 0;
	do {
		char buf[4096];
		int len = std::fread(buf, sizeof(char), 4096, fh);

		if (character->world->config["GlobalPK"] && !character->world->PKExcept(character->mapid))
		{
			if (p + len >= 0x04 && 0x03 - p > 0) buf[0x03 - p] = 0xFF;
			if (p + len >= 0x05 && 0x04 - p > 0) buf[0x04 - p] = 0x01;
			if (p + len >= 0x20 && 0x1F - p > 0) buf[0x1F - p] = 0x04;
		}

		p += len;
		content.append(buf, len);
	} while (!std::feof(fh));

	std::fclose(fh);

	PacketBuilder reply(0);
	reply.AddChar(INIT_BANNED); // wtf? When in Rome...
	reply.AddString(content);
	character->Send(reply);

	if (character->world->config["ProtectMaps"])
	{
		reply.Reset();
		reply.AddChar(INIT_BANNED);
		character->Send(reply);
	}
}

PACKET_HANDLER_REGISTER(PACKET_WARP)
	Register(PACKET_ACCEPT, Warp_Accept, Playing);
	Register(PACKET_TAKE, Warp_Take, Playing);
PACKET_HANDLER_REGISTER_END()

}
