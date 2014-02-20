
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../eoplus.hpp"
#include "../guild.hpp"
#include "../quest.hpp"

namespace Handlers
{

// User requests another's Book (Quest history)
void Book_Request(Character *character, PacketReader &reader)
{
	unsigned int id = reader.GetShort();

	Character *target = character->map->GetCharacterPID(id);

	if (!target)
	{
		target = character;
	}

	std::string home_str = target->world->GetHome(target)->name;
	std::string guild_str = target->guild ? target->guild->name : "";
	std::string rank_str = target->guild ? target->guild->GetRank(target->guild_rank) : "";

	PacketBuilder reply(PACKET_BOOK, PACKET_REPLY,
		13 + target->name.length() + home_str.length() + target->partner.length() + target->title.length()
		+ guild_str.length() + rank_str.length());

	reply.AddBreakString(target->name);
	reply.AddBreakString(home_str);
	reply.AddBreakString(target->partner);
	reply.AddBreakString(target->title);
	reply.AddBreakString(guild_str);
	reply.AddBreakString(rank_str);
	reply.AddShort(id);
	reply.AddChar(target->clas);
	reply.AddChar(target->gender);
	reply.AddChar(0);

	if (target->admin >= ADMIN_HGM)
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
	else if (target->admin >= ADMIN_GUIDE)
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

	reply.AddByte(255);

	std::size_t reserve = 0;

	UTIL_FOREACH(target->quests, quest)
	{
		if (quest.second->Finished() && quest.second->GetQuest()->GetQuest()->info.hidden == EOPlus::Info::NotHidden)
			reserve += quest.second->GetQuest()->Name().length() + 1;
	}

	reply.ReserveMore(reserve);

	UTIL_FOREACH(target->quests, quest)
	{
		if (quest.second->Finished() && quest.second->GetQuest()->GetQuest()->info.hidden == EOPlus::Info::NotHidden)
			reply.AddBreakString(quest.second->GetQuest()->Name());
	}

	character->Send(reply);
}

PACKET_HANDLER_REGISTER(PACKET_BOOK)
	Register(PACKET_REQUEST, Book_Request, Playing);
PACKET_HANDLER_REGISTER_END(PACKET_BOOK)

}
