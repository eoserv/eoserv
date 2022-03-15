/* handlers/Book.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../eoplus.hpp"
#include "../quest.hpp"

#include "../util.hpp"

#include <cstddef>
#include <string>

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

	std::string home_str = target->HomeString();
	std::string guild_str = target->GuildNameString();
	std::string rank_str = target->GuildRankString();

	PacketBuilder reply(PACKET_BOOK, PACKET_REPLY,
		13 + target->SourceName().length() + home_str.length() + target->partner.length() + target->title.length()
		+ guild_str.length() + rank_str.length());

	reply.AddBreakString(target->SourceName());
	reply.AddBreakString(home_str);
	reply.AddBreakString(target->partner);
	reply.AddBreakString(target->title);
	reply.AddBreakString(guild_str);
	reply.AddBreakString(rank_str);
	reply.AddShort(id);
	reply.AddChar(target->clas);
	reply.AddChar(target->gender);
	reply.AddChar(0);

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

	reply.AddByte(255);

	std::size_t reserve = 0;

	UTIL_FOREACH(target->quests, quest)
	{
		if (quest.second && quest.second->Finished() && !quest.second->IsHidden())
			reserve += quest.second->GetQuest()->Name().length() + 1;
	}

	reply.ReserveMore(reserve);

	UTIL_FOREACH(target->quests, quest)
	{
		if (quest.second && quest.second->Finished() && !quest.second->IsHidden())
			reply.AddBreakString(quest.second->GetQuest()->Name());
	}

	character->Send(reply);
}

PACKET_HANDLER_REGISTER(PACKET_BOOK)
	Register(PACKET_REQUEST, Book_Request, Playing);
PACKET_HANDLER_REGISTER_END(PACKET_BOOK)

}
