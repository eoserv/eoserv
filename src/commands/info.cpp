
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "commands.hpp"

#include "../util.hpp"

#include "../packet.hpp"
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
				default: ;
			}

			reply.AddString(" ");
			reply.AddBreakString(util::trim(victim->PaddedGuildTag()));
			reply.AddInt(victim->Usage());
			reply.AddByte(255);
			reply.AddInt(victim->goldbank);
			reply.AddByte(255);

			UTIL_CFOREACH(victim->inventory, item)
			{
				reply.AddShort(item.id);
				reply.AddInt(item.amount);
			}
			reply.AddByte(255);

			UTIL_CFOREACH(victim->bank, item)
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

COMMAND_HANDLER_REGISTER()
	RegisterCharacter({"inventory", {}, {}, 3}, Inventory);
	RegisterCharacter({"info", {}, {}, 2}, Info);
COMMAND_HANDLER_REGISTER_END()

}
