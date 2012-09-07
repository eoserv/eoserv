
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "commands.hpp"

#include <functional>

#include "../arena.hpp"
#include "../character.hpp"
#include "../config.hpp"
#include "../map.hpp"
#include "../packet.hpp"
#include "../player.hpp"
#include "../quest.hpp"
#include "../world.hpp"

namespace Commands
{

void SetX(const std::vector<std::string>& arguments, Command_Source* from, std::string set)
{
	Character *victim = from->SourceWorld()->GetCharacter(arguments[0]);

	if (!victim)
	{
		from->ServerMsg(from->SourceWorld()->i18n.Format("character_not_found"));
	}
	else if (victim->admin >= from->SourceAccess() && victim != from->SourceCharacter())
	{
		from->ServerMsg(from->SourceWorld()->i18n.Format("command_access_denied"));
	}
	else
	{
		std::string title_string = "";

		for (std::size_t i = 1; i < arguments.size(); ++i)
		{
			title_string += arguments[i];

			if (i < arguments.size() - 1)
				title_string += " ";
		}

		bool appearance = false;
		bool failure = false;
		bool level = false;
		bool stats = false;
		bool karma = false;

		bool statpoints = false;
		bool skillpoints = false;

			 if (set == "level") (level = true, victim->level) = std::min(std::max(util::to_int(arguments[1]), 0), int(from->SourceWorld()->config["MaxLevel"]));
		else if (set == "exp") (level = true, victim->exp) = std::min(std::max(util::to_int(arguments[1]), 0), int(from->SourceWorld()->config["MaxEXP"]));
		else if (set == "str") (stats = true, victim->str) = std::min(std::max(util::to_int(arguments[1]), 0), int(from->SourceWorld()->config["MaxStat"]));
		else if (set == "int") (stats = true, victim->intl) = std::min(std::max(util::to_int(arguments[1]), 0), int(from->SourceWorld()->config["MaxStat"]));
		else if (set == "wis") (stats = true, victim->wis) = std::min(std::max(util::to_int(arguments[1]), 0), int(from->SourceWorld()->config["MaxStat"]));
		else if (set == "agi") (stats = true, victim->agi) = std::min(std::max(util::to_int(arguments[1]), 0), int(from->SourceWorld()->config["MaxStat"]));
		else if (set == "con") (stats = true, victim->con) = std::min(std::max(util::to_int(arguments[1]), 0), int(from->SourceWorld()->config["MaxStat"]));
		else if (set == "cha") (stats = true, victim->cha) = std::min(std::max(util::to_int(arguments[1]), 0), int(from->SourceWorld()->config["MaxStat"]));
		else if (set == "statpoints") (statpoints = true, victim->statpoints) = std::min(std::max(util::to_int(arguments[1]), 0), int(from->SourceWorld()->config["MaxLevel"]) * int(from->SourceWorld()->config["StatPerLevel"]));
		else if (set == "skillpoints") (skillpoints = true, victim->skillpoints) = std::min(std::max(util::to_int(arguments[1]), 0), int(from->SourceWorld()->config["MaxLevel"]) * int(from->SourceWorld()->config["SkillPerLevel"]));
		else if (set == "admin")
		{
			AdminLevel level = std::min(std::max(AdminLevel(util::to_int(arguments[1])), ADMIN_PLAYER), ADMIN_HGM);

			if (level < from->SourceAccess() && victim != from->SourceCharacter())
			{
				if (level == ADMIN_PLAYER && victim->admin != ADMIN_PLAYER)
					victim->world->DecAdminCount();
				else if (level != ADMIN_PLAYER && victim->admin == ADMIN_PLAYER)
					victim->world->IncAdminCount();

				victim->admin = level;
			}
			else
			{
				from->ServerMsg(from->SourceWorld()->i18n.Format("command_access_denied"));
			}
		}
		else if (set == "title") victim->title = title_string;
		else if (set == "fiance") victim->fiance = (arguments.size() > 1) ? arguments[1] : "";
		else if (set == "partner") victim->partner = (arguments.size() > 1) ? arguments[1] : "";
		else if (set == "home") victim->home = (arguments.size() > 1) ? arguments[1] : "";
		else if (set == "gender") (appearance = true, victim->gender) = Gender(std::min(std::max(util::to_int(arguments[1]), 0), 1));
		else if (set == "hairstyle") (appearance = true, victim->hairstyle) = std::min(std::max(util::to_int(arguments[1]), 0), int(from->SourceWorld()->config["MaxHairStyle"]));
		else if (set == "haircolor") (appearance = true, victim->haircolor) = std::min(std::max(util::to_int(arguments[1]), 0), int(from->SourceWorld()->config["MaxHairColor"]));
		else if (set == "race") (appearance = true, victim->race) = Skin(std::min(std::max(util::to_int(arguments[1]), 0), int(from->SourceWorld()->config["MaxSkin"])));
		else if (set == "guildrank") victim->guild_rank = std::min(std::max(util::to_int(arguments[1]), 0), 9);
		else if (set == "karma") (karma = true, victim->karma) = std::min(std::max(util::to_int(arguments[1]), 0), 2000);
		else if (set == "class") (stats = true, victim->clas) = std::min(std::max(util::to_int(arguments[1]), 0), int(from->SourceWorld()->ecf->data.size() - 1));
		else failure = true;

		if (failure)
		{
			from->ServerMsg(from->SourceWorld()->i18n.Format("invalid_setx"));
		}
		else
		{
			// Easiest way to get the character to update on everyone nearby's screen
			if (appearance)
				victim->Warp(victim->map->id, victim->x, victim->y);

			// TODO: Good way of updating skillpoints
			(void)skillpoints;

			if (stats || statpoints)
			{
				victim->CalculateStats();

				PacketBuilder builder(PACKET_RECOVER, PACKET_LIST, 32);

				if (statpoints)
				{
					builder.SetID(PACKET_STATSKILL, PACKET_PLAYER);
					builder.AddShort(victim->statpoints);
				}
				else
				{
					builder.AddShort(victim->clas);
				}

				builder.AddShort(victim->display_str);
				builder.AddShort(victim->display_intl);
				builder.AddShort(victim->display_wis);
				builder.AddShort(victim->display_agi);
				builder.AddShort(victim->display_con);
				builder.AddShort(victim->display_cha);
				builder.AddShort(victim->maxhp);
				builder.AddShort(victim->maxtp);
				builder.AddShort(victim->maxsp);
				builder.AddShort(victim->maxweight);
				builder.AddShort(victim->mindam);
				builder.AddShort(victim->maxdam);
				builder.AddShort(victim->accuracy);
				builder.AddShort(victim->evade);
				builder.AddShort(victim->armor);
				victim->Send(builder);
			}

			if (karma || level)
			{
				PacketBuilder builder(PACKET_RECOVER, PACKET_REPLY, 7);
				builder.AddInt(victim->exp);
				builder.AddShort(victim->karma);
				builder.AddChar(level ? victim->level : 0);
				victim->Send(builder);
			}

			if (!stats && !skillpoints && !appearance)
			{
				victim->CheckQuestRules();
			}
		}
	}
}

void Strip(const std::vector<std::string>& arguments, Command_Source* from)
{
	Character *victim = from->SourceWorld()->GetCharacter(arguments[0]);

	if (!victim)
	{
		from->ServerMsg(from->SourceWorld()->i18n.Format("character_not_found"));
	}
	else
	{
		if (victim->admin < from->SourceAccess() || victim == from->SourceCharacter())
		{
			for (std::size_t i = 0; i < victim->paperdoll.size(); ++i)
			{
				if (victim->paperdoll[i] != 0)
				{
					int itemid = victim->paperdoll[i];
					int subloc = ((i == Character::Ring2 || i == Character::Armlet2 || i == Character::Bracer2) ? 1 : 0);

					if (victim->Unequip(victim->paperdoll[i], subloc))
					{
						PacketBuilder builder(PACKET_PAPERDOLL, PACKET_REMOVE, 43);
						builder.AddShort(victim->player->id);
						builder.AddChar(SLOT_CLOTHES);
						builder.AddChar(0); // ?
						builder.AddShort(victim->world->eif->Get(victim->paperdoll[Character::Boots])->dollgraphic);
						builder.AddShort(victim->world->eif->Get(victim->paperdoll[Character::Armor])->dollgraphic);
						builder.AddShort(victim->world->eif->Get(victim->paperdoll[Character::Hat])->dollgraphic);
						builder.AddShort(victim->world->eif->Get(victim->paperdoll[Character::Weapon])->dollgraphic);
						builder.AddShort(victim->world->eif->Get(victim->paperdoll[Character::Shield])->dollgraphic);
						builder.AddShort(itemid);
						builder.AddChar(subloc);
						builder.AddShort(victim->maxhp);
						builder.AddShort(victim->maxtp);
						builder.AddShort(victim->display_str);
						builder.AddShort(victim->display_intl);
						builder.AddShort(victim->display_wis);
						builder.AddShort(victim->display_agi);
						builder.AddShort(victim->display_con);
						builder.AddShort(victim->display_cha);
						builder.AddShort(victim->mindam);
						builder.AddShort(victim->maxdam);
						builder.AddShort(victim->accuracy);
						builder.AddShort(victim->evade);
						builder.AddShort(victim->armor);
						victim->Send(builder);
					}
				}
			}

			PacketBuilder builder(PACKET_AVATAR, PACKET_AGREE, 14);
			builder.AddShort(victim->player->id);
			builder.AddChar(SLOT_CLOTHES);
			builder.AddChar(0);
			builder.AddShort(0);
			builder.AddShort(0);
			builder.AddShort(0);
			builder.AddShort(0);
			builder.AddShort(0);

			UTIL_FOREACH(victim->map->characters, updatecharacter)
			{
				if (updatecharacter == victim || !victim->InRange(updatecharacter))
					continue;

				updatecharacter->Send(builder);
			}
		}
		else
		{
			from->ServerMsg(from->SourceWorld()->i18n.Format("command_access_denied"));
		}
	}
}

COMMAND_HANDLER_REGISTER()
	using namespace std::placeholders;
	Register({"setlevel", {"victim", "value"}, {}, 4}, std::bind(SetX, _1, _2, "level"));
	Register({"setexp", {"victim", "value"}, {}, 4}, std::bind(SetX, _1, _2, "exp"));
	Register({"setstr", {"victim", "value"}, {}, 4}, std::bind(SetX, _1, _2, "str"));
	Register({"setint", {"victim", "value"}, {}, 4}, std::bind(SetX, _1, _2, "int"));
	Register({"setwis", {"victim", "value"}, {}, 4}, std::bind(SetX, _1, _2, "wis"));
	Register({"setagi", {"victim", "value"}, {}, 4}, std::bind(SetX, _1, _2, "agi"));
	Register({"setcon", {"victim", "value"}, {}, 4}, std::bind(SetX, _1, _2, "con"));
	Register({"setcha", {"victim", "value"}, {}, 4}, std::bind(SetX, _1, _2, "cha"));
	Register({"setstatpoints", {"victim", "value"}, {}, 7}, std::bind(SetX, _1, _2, "statpoints"));
	Register({"setskillpoints", {"victim", "value"}, {}, 8}, std::bind(SetX, _1, _2, "skillpoints"));
	Register({"setadmin", {"victim", "value"}, {}, 8}, std::bind(SetX, _1, _2, "admin"));
	Register({"settitle", {"victim", "value"}, {}, 4}, std::bind(SetX, _1, _2, "title"));
	Register({"setfiance", {"victim", "value"}, {}, 4}, std::bind(SetX, _1, _2, "fiance"));
	Register({"setpartner", {"victim", "value"}, {}, 4}, std::bind(SetX, _1, _2, "partner"));
	Register({"sethome", {"victim", "value"}, {}, 4}, std::bind(SetX, _1, _2, "home"));
	Register({"setgender", {"victim", "value"}, {}, 4}, std::bind(SetX, _1, _2, "gender"));
	Register({"sethairstyle", {"victim", "value"}, {}, 8}, std::bind(SetX, _1, _2, "hairstyle"));
	Register({"sethaircolor", {"victim", "value"}, {}, 8}, std::bind(SetX, _1, _2, "haircolor"));
	Register({"setrace", {"victim", "value"}, {}, 4}, std::bind(SetX, _1, _2, "race"));
	Register({"setguildrank", {"victim", "value"}, {}, 4}, std::bind(SetX, _1, _2, "guildrank"));
	Register({"setkarma", {"victim", "value"}, {}, 4}, std::bind(SetX, _1, _2, "karma"));
	Register({"setclass", {"victim", "value"}, {}, 4}, std::bind(SetX, _1, _2, "class"));
	Register({"strip", {"victim"}, {}, 2}, Strip);
COMMAND_HANDLER_REGISTER_END()

}
