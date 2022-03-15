/* handlers/Paperdoll.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../eodata.hpp"
#include "../map.hpp"
#include "../packet.hpp"
#include "../world.hpp"

#include "../util.hpp"

#include <string>

namespace Handlers
{

// Request for currently equipped items
void Paperdoll_Request(Character *character, PacketReader &reader)
{
	unsigned int id = reader.GetShort();

	Character *target = character->map->GetCharacterPID(id);

	if (!target)
	{
		target = character;
	}

	std::string home_str = target->HomeString();
	std::string guild_str = target->GuildNameString();
	std::string rank_str = target->GuildRankString();

	PacketBuilder reply(PACKET_PAPERDOLL, PACKET_REPLY,
		12 + target->SourceName().length() + home_str.length() + target->partner.length() + target->title.length()
		+ guild_str.length() + rank_str.length() + target->paperdoll.size() * 2);

	reply.AddBreakString(target->SourceName());
	reply.AddBreakString(home_str);
	reply.AddBreakString(target->partner);
	reply.AddBreakString(target->title);
	reply.AddBreakString(guild_str);
	reply.AddBreakString(rank_str);
	reply.AddShort(target->PlayerID());
	reply.AddChar(target->clas);
	reply.AddChar(target->gender);
	reply.AddChar(0);

	UTIL_FOREACH(target->paperdoll, item)
	{
		reply.AddShort(item);
	}

	if (target->admin >= ADMIN_HGM && !target->IsHideAdmin())
	{
		if (target->party)
		{
			reply.AddChar(ICON_HGM_PARTY);
		}
		else
		{
			reply.AddChar(ICON_HGM);
		}
	}
	else if (target->admin >= ADMIN_GUIDE && !target->IsHideAdmin())
	{
		if (target->party)
		{
			reply.AddChar(ICON_GM_PARTY);
		}
		else
		{
			reply.AddChar(ICON_GM);
		}
	}
	else
	{
		if (target->party)
		{
			reply.AddChar(ICON_PARTY);
		}
		else
		{
			reply.AddChar(ICON_NORMAL);
		}
	}

	character->Send(reply);
}

// Unequipping an item
void Paperdoll_Remove(Character *character, PacketReader &reader)
{
	int itemid = reader.GetShort();
	int subloc = reader.GetChar(); // Used for double slot items (rings etc)

	if (character->world->eif->Get(itemid).special == EIF::Cursed)
	{
		return;
	}

	if (character->Unequip(itemid, subloc))
	{
		PacketBuilder reply(PACKET_PAPERDOLL, PACKET_REMOVE, 43);
		reply.AddShort(character->PlayerID());
		reply.AddChar(SLOT_CLOTHES);
		reply.AddChar(0); // sound
		character->AddPaperdollData(reply, "BAHWS");

		reply.AddShort(itemid);
		reply.AddChar(subloc);
		reply.AddShort(character->maxhp);
		reply.AddShort(character->maxtp);
		reply.AddShort(character->display_str);
		reply.AddShort(character->display_intl);
		reply.AddShort(character->display_wis);
		reply.AddShort(character->display_agi);
		reply.AddShort(character->display_con);
		reply.AddShort(character->display_cha);
		reply.AddShort(character->mindam);
		reply.AddShort(character->maxdam);
		reply.AddShort(character->accuracy);
		reply.AddShort(character->evade);
		reply.AddShort(character->armor);
		character->Send(reply);
	}
	// TODO: Only send this if they change a viewable item

	PacketBuilder builder(PACKET_AVATAR, PACKET_AGREE, 14);
	builder.AddShort(character->PlayerID());
	builder.AddChar(SLOT_CLOTHES);
	builder.AddChar(0); // sound
	character->AddPaperdollData(builder, "BAHWS");

	UTIL_FOREACH(character->map->characters, updatecharacter)
	{
		if (updatecharacter == character || !character->InRange(updatecharacter))
			continue;

		updatecharacter->Send(builder);
	}
}

// Equipping an item
void Paperdoll_Add(Character *character, PacketReader &reader)
{
	if (character->trading) return;

	int itemid = reader.GetShort();
	int subloc = reader.GetChar(); // Used for double slot items (rings etc)

	// TODO: Find out if we can handle equipping items when we have more than 16.7m of them better

	if (character->Equip(itemid, subloc))
	{
		PacketBuilder reply(PACKET_PAPERDOLL, PACKET_AGREE, 46);
		reply.AddShort(character->PlayerID());
		reply.AddChar(SLOT_CLOTHES);
		reply.AddChar(0); // sound
		character->AddPaperdollData(reply, "BAHWS");

		reply.AddShort(itemid);
		reply.AddThree(character->HasItem(itemid));
		reply.AddChar(subloc);
		reply.AddShort(character->maxhp);
		reply.AddShort(character->maxtp);
		reply.AddShort(character->display_str);
		reply.AddShort(character->display_intl);
		reply.AddShort(character->display_wis);
		reply.AddShort(character->display_agi);
		reply.AddShort(character->display_con);
		reply.AddShort(character->display_cha);
		reply.AddShort(character->mindam);
		reply.AddShort(character->maxdam);
		reply.AddShort(character->accuracy);
		reply.AddShort(character->evade);
		reply.AddShort(character->armor);
		character->Send(reply);
	}

	// TODO: Only send this if they change a viewable item

	PacketBuilder builder(PACKET_AVATAR, PACKET_AGREE, 14);
	builder.AddShort(character->PlayerID());
	builder.AddChar(SLOT_CLOTHES);
	builder.AddChar(0); // sound
	character->AddPaperdollData(builder, "BAHWS");

	UTIL_FOREACH(character->map->characters, updatecharacter)
	{
		if (updatecharacter == character || !character->InRange(updatecharacter))
			continue;

		updatecharacter->Send(builder);
	}
}

PACKET_HANDLER_REGISTER(PACKET_PAPERDOLL)
	Register(PACKET_REQUEST, Paperdoll_Request, Playing);
	Register(PACKET_REMOVE, Paperdoll_Remove, Playing);
	Register(PACKET_ADD, Paperdoll_Add, Playing);
PACKET_HANDLER_REGISTER_END(PACKET_PAPERDOLL)

}
