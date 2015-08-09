
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "commands.hpp"

#include "../util.hpp"

#include "../eoplus.hpp"
#include "../packet.hpp"
#include "../player.hpp"
#include "../quest.hpp"
#include "../world.hpp"

namespace Commands
{

void Inventory(const std::vector<std::string>& arguments, Character* from)
{
	Character *victim = from->world->GetCharacter(arguments[0]);

	if (!victim)
	{
		from->ServerMsg(from->world->i18n.Format("character_not_found"));
	}
	else
	{
		if (victim->admin < int(from->world->admin_config["cmdprotect"]) || victim->admin <= from->SourceAccess())
		{
			std::string name = util::ucfirst(victim->name);

			PacketBuilder reply(PACKET_ADMININTERACT, PACKET_LIST, 32 + name.length() + victim->inventory.size() * 6 + victim->bank.size() * 7);

			switch (victim->admin)
			{
				case ADMIN_HGM: reply.AddString(from->world->i18n.Format("high_game_master_title", name)); break;
				case ADMIN_GM: reply.AddString(from->world->i18n.Format("game_master_title", name)); break;
				case ADMIN_GUARDIAN: reply.AddString(from->world->i18n.Format("guardian_title", name)); break;
				case ADMIN_GUIDE: reply.AddString(from->world->i18n.Format("light_guide_title", name)); break;
				default: reply.AddString(name); break;
			}

			reply.AddString(" ");
			reply.AddBreakString(util::trim(victim->PaddedGuildTag()));
			reply.AddInt(victim->Usage());
			reply.AddByte(255);
			reply.AddInt(victim->goldbank);
			reply.AddByte(255);

			UTIL_FOREACH(victim->inventory, item)
			{
				reply.AddShort(item.id);
				reply.AddInt(item.amount);
			}
			reply.AddByte(255);

			UTIL_FOREACH(victim->bank, item)
			{
				reply.AddShort(item.id);
				reply.AddThree(item.amount);
			}

			from->Send(reply);
		}
		else
		{
			from->ServerMsg(from->world->i18n.Format("command_access_denied"));
		}
	}
}

void Paperdoll(const std::vector<std::string>& arguments, Character* from)
{
	Character *victim = from->world->GetCharacter(arguments[0]);

	if (!victim)
	{
		from->ServerMsg(from->world->i18n.Format("character_not_found"));
	}
	else
	{
		std::string home_str = victim->world->GetHome(victim)->name;
		std::string guild_str = victim->guild ? victim->guild->name : "";
		std::string rank_str = victim->guild ? victim->guild->GetRank(victim->guild_rank) : "";

		PacketBuilder reply(PACKET_PAPERDOLL, PACKET_REPLY,
			12 + victim->name.length() + home_str.length() + victim->partner.length() + victim->title.length()
			+ guild_str.length() + rank_str.length() + victim->paperdoll.size() * 2);

		reply.AddBreakString(victim->name);
		reply.AddBreakString(home_str);
		reply.AddBreakString(victim->partner);
		reply.AddBreakString(victim->title);
		reply.AddBreakString(guild_str);
		reply.AddBreakString(rank_str);
		reply.AddShort(victim->player->id);
		reply.AddChar(victim->clas);
		reply.AddChar(victim->gender);
		reply.AddChar(0);

		UTIL_FOREACH(victim->paperdoll, item)
		{
			reply.AddShort(item);
		}

		if (victim->admin >= ADMIN_HGM && !victim->IsHideAdmin())
		{
			if (victim->party)
			{
				reply.AddChar(ICON_HGM_PARTY);
			}
			else
			{
				reply.AddChar(ICON_HGM);
			}
		}
		else if (victim->admin >= ADMIN_GUIDE && !victim->IsHideAdmin())
		{
			if (victim->party)
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
			if (victim->party)
			{
				reply.AddChar(ICON_PARTY);
			}
			else
			{
				reply.AddChar(ICON_NORMAL);
			}
		}

		from->Send(reply);
	}
}

void Book(const std::vector<std::string>& arguments, Character* from)
{
	Character *victim = from->world->GetCharacter(arguments[0]);

	if (!victim)
	{
		from->ServerMsg(from->world->i18n.Format("character_not_found"));
	}
	else
	{
		std::string home_str = victim->world->GetHome(victim)->name;
		std::string guild_str = victim->guild ? victim->guild->name : "";
		std::string rank_str = victim->guild ? victim->guild->GetRank(victim->guild_rank) : "";

		PacketBuilder reply(PACKET_BOOK, PACKET_REPLY,
			13 + victim->name.length() + home_str.length() + victim->partner.length() + victim->title.length()
			+ guild_str.length() + rank_str.length());

		reply.AddBreakString(victim->name);
		reply.AddBreakString(home_str);
		reply.AddBreakString(victim->partner);
		reply.AddBreakString(victim->title);
		reply.AddBreakString(guild_str);
		reply.AddBreakString(rank_str);
		reply.AddShort(victim->player->id);
		reply.AddChar(victim->clas);
		reply.AddChar(victim->gender);
		reply.AddChar(0);

		if (victim->admin >= ADMIN_HGM)
		{
			if (victim->party)
			{
				reply.AddChar(ICON_HGM_PARTY);
			}
			else
			{
				reply.AddChar(ICON_HGM);
			}
		}
		else if (victim->admin >= ADMIN_GUIDE)
		{
			if (victim->party)
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
			if (victim->party)
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

		UTIL_FOREACH(victim->quests, quest)
		{
			if (quest.second && quest.second->Finished() && quest.second->GetQuest()->GetQuest()->info.hidden == EOPlus::Info::NotHidden)
				reserve += quest.second->GetQuest()->Name().length() + 1;
		}

		reply.ReserveMore(reserve);

		UTIL_FOREACH(victim->quests, quest)
		{
			if (quest.second && quest.second->Finished() && quest.second->GetQuest()->GetQuest()->info.hidden == EOPlus::Info::NotHidden)
				reply.AddBreakString(quest.second->GetQuest()->Name());
		}

		from->Send(reply);
	}
}

void Info(const std::vector<std::string>& arguments, Character* from)
{
	Character *victim = from->world->GetCharacter(arguments[0]);

	if (!victim)
	{
		from->ServerMsg(from->world->i18n.Format("character_not_found"));
	}
	else
	{
		if (victim->admin < int(from->world->admin_config["cmdprotect"]) || victim->admin <= from->SourceAccess())
		{
			std::string name = util::ucfirst(victim->name);

			PacketBuilder reply(PACKET_ADMININTERACT, PACKET_TELL, 85 + name.length());

			switch (victim->admin)
			{
				case ADMIN_HGM: reply.AddString(from->world->i18n.Format("high_game_master_title", name)); break;
				case ADMIN_GM: reply.AddString(from->world->i18n.Format("game_master_title", name)); break;
				case ADMIN_GUARDIAN: reply.AddString(from->world->i18n.Format("guardian_title", name)); break;
				case ADMIN_GUIDE: reply.AddString(from->world->i18n.Format("light_guide_title", name)); break;
				default: ;
			}

			reply.AddString(" ");
			reply.AddBreakString(util::trim(victim->PaddedGuildTag()));
			reply.AddInt(victim->Usage());
			reply.AddByte(255);
			reply.AddByte(255);
			reply.AddInt(victim->exp);
			reply.AddChar(victim->level);
			reply.AddShort(victim->mapid);
			reply.AddShort(victim->x);
			reply.AddShort(victim->y);
			reply.AddShort(victim->hp);
			reply.AddShort(victim->maxhp);
			reply.AddShort(victim->tp);
			reply.AddShort(victim->maxtp);
			reply.AddShort(victim->display_str);
			reply.AddShort(victim->display_intl);
			reply.AddShort(victim->display_wis);
			reply.AddShort(victim->display_agi);
			reply.AddShort(victim->display_con);
			reply.AddShort(victim->display_cha);
			reply.AddShort(victim->maxdam);
			reply.AddShort(victim->mindam);
			reply.AddShort(victim->accuracy);
			reply.AddShort(victim->evade);
			reply.AddShort(victim->armor);
			reply.AddShort(0); // light
			reply.AddShort(0); // dark
			reply.AddShort(0); // fire
			reply.AddShort(0); // water
			reply.AddShort(0); // earth
			reply.AddShort(0); // wind
			reply.AddChar(victim->weight);
			reply.AddChar(victim->maxweight);

			from->Send(reply);
		}
		else
		{
			from->ServerMsg(from->world->i18n.Format("command_access_denied"));
		}
	}
}

COMMAND_HANDLER_REGISTER(info)
	RegisterCharacter({"inventory", {"victim"}, {}, 3}, Inventory);
	RegisterCharacter({"paperdoll", {"victim"}, {}}, Paperdoll);
	RegisterCharacter({"book", {"victim"}, {}, 2}, Book);
	RegisterCharacter({"info", {"victim"}, {}}, Info);

	RegisterAlias("p", "paperdoll");
	RegisterAlias("i", "info");
COMMAND_HANDLER_REGISTER_END(info)

}
