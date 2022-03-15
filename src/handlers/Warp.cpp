/* handlers/Warp.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../eoclient.hpp"
#include "../map.hpp"
#include "../npc.hpp"
#include "../player.hpp"

#include "../util.hpp"

#include <memory>
#include <stdexcept>
#include <vector>

namespace Handlers
{

// Player accepting a warp request from the server
void Warp_Accept(Character *character, PacketReader &reader)
{
	//int map = reader.GetShort();
	(void)reader;

	int anim = character->warp_anim;

	// Removed (character->mapid == map) check as it interferes with fallback maps

	if (anim != WARP_ANIMATION_INVALID)
	{
		character->warp_anim = WARP_ANIMATION_INVALID;
	}
	else
	{
		return;
	}

	std::vector<Character *> updatecharacters;
	std::vector<NPC *> updatenpcs;
	std::vector<std::shared_ptr<Map_Item>> updateitems;

	UTIL_FOREACH(character->map->characters, checkcharacter)
	{
		if (checkcharacter->InRange(character))
		{
			updatecharacters.push_back(checkcharacter);
		}
	}

	UTIL_FOREACH(character->map->npcs, npc)
	{
		if (character->InRange(npc) && npc->alive)
		{
			updatenpcs.push_back(npc);
		}
	}

	UTIL_FOREACH(character->map->items, item)
	{
		if (character->InRange(*item))
		{
			updateitems.push_back(item);
		}
	}

	PacketBuilder reply(PACKET_WARP, PACKET_AGREE,
		7 + updatecharacters.size() * 60 + updatenpcs.size() * 6 + updateitems.size() * 9);

	reply.AddChar(2); // ?
	reply.AddShort(character->mapid);
	reply.AddChar(anim);
	reply.AddChar(updatecharacters.size());
	reply.AddByte(255);
	UTIL_FOREACH(updatecharacters, character)
	{
		reply.AddBreakString(character->SourceName());
		reply.AddShort(character->PlayerID());
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
		character->AddPaperdollData(reply, "B000A0HSW");

		reply.AddChar(character->sitting);
		reply.AddChar(character->IsHideInvisible());
		reply.AddByte(255);
	}
	UTIL_FOREACH(updatenpcs, npc)
	{
		reply.AddChar(npc->index);
		reply.AddShort(npc->id);
		reply.AddChar(npc->x);
		reply.AddChar(npc->y);
		reply.AddChar(npc->direction);
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

	if (!character->player->client->Upload(FILE_MAP, character->mapid, INIT_BANNED))
		throw std::runtime_error("Failed to upload file");
}

PACKET_HANDLER_REGISTER(PACKET_WARP)
	Register(PACKET_ACCEPT, Warp_Accept, Playing);
	Register(PACKET_TAKE, Warp_Take, Playing, 0.15);
PACKET_HANDLER_REGISTER_END(PACKET_WARP)

}
