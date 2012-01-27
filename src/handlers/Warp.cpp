
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include <stdexcept>
#include <vector>

#include "../character.hpp"
#include "../console.hpp"
#include "../eoclient.hpp"
#include "../eodata.hpp"
#include "../map.hpp"
#include "../npc.hpp"
#include "../player.hpp"

namespace Handlers
{

// Player accepting a warp request from the server
void Warp_Accept(Character *character, PacketReader &reader)
{
	/*int map = */reader.GetShort();

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
		if (character->InRange(npc) && npc->alive)
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

	PacketBuilder reply(PACKET_WARP, PACKET_AGREE,
		7 + updatecharacters.size() * 60 + updatenpcs.size() * 6 + updateitems.size() * 9);

	reply.AddChar(2); // ?
	reply.AddShort(character->mapid);
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

		EIF_Data* wep = character->world->eif->Get(character->paperdoll[Character::Weapon]);

		if (wep->subtype == EIF::TwoHanded && wep->dual_wield_dollgraphic)
			reply.AddShort(wep->dual_wield_dollgraphic);
		else
			reply.AddShort(character->world->eif->Get(character->paperdoll[Character::Shield])->dollgraphic);

		reply.AddShort(wep->dollgraphic);

		reply.AddChar(character->sitting);
		reply.AddChar(character->hidden);
		reply.AddByte(255);
	}
	UTIL_FOREACH(updatenpcs, npc)
	{
		reply.AddChar(npc->index);
		reply.AddShort(npc->Data()->id);
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
	Register(PACKET_TAKE, Warp_Take, Playing);
PACKET_HANDLER_REGISTER_END()

}
