
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "commands.hpp"

#include "../util.hpp"

#include "../map.hpp"
#include "../packet.hpp"
#include "../world.hpp"

namespace Commands
{

void Warp(const std::vector<std::string>& arguments, Character* from)
{
	int map = util::to_int(arguments[0]);
	int x = util::to_int(arguments[1]);
	int y = util::to_int(arguments[2]);

	from->Warp(map, x, y, from->world->config["WarpBubbles"] ? WARP_ANIMATION_ADMIN : WARP_ANIMATION_NONE);
}

void WarpMeTo(const std::vector<std::string>& arguments, Character* from)
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
			from->Warp(victim->mapid, victim->x, victim->y, from->world->config["WarpBubbles"] ? WARP_ANIMATION_ADMIN : WARP_ANIMATION_NONE);
		}
		else
		{
			from->ServerMsg(from->world->i18n.Format("command_access_denied"));
		}
	}
}

void WarpToMe(const std::vector<std::string>& arguments, Character* from)
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
			victim->Warp(from->mapid, from->x, from->y, from->world->config["WarpBubbles"] ? WARP_ANIMATION_ADMIN : WARP_ANIMATION_NONE);
		}
		else
		{
			from->ServerMsg(from->world->i18n.Format("command_access_denied"));
		}
	}
}

COMMAND_HANDLER_REGISTER(warp)
	RegisterCharacter({"warp", {"map", "x", "y"}}, Warp);
	RegisterCharacter({"warpmeto", {"victim"}}, WarpMeTo);
	RegisterCharacter({"warptome", {"victim"}}, WarpToMe);
	RegisterAlias("w", "warp");
COMMAND_HANDLER_REGISTER_END(warp)

}
