/* commands/warp.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "commands.hpp"

#include "../character.hpp"
#include "../config.hpp"
#include "../i18n.hpp"
#include "../map.hpp"
#include "../world.hpp"

#include "../util.hpp"

#include <string>
#include <vector>

namespace Commands
{

void Warp(const std::vector<std::string>& arguments, Character* from)
{
	int map = util::to_int(arguments[0]);
	int x = util::to_int(arguments[1]);
	int y = util::to_int(arguments[2]);

	bool bubbles = from->world->config["WarpBubbles"] && !from->IsHideWarp();
	from->Warp(map, x, y, bubbles ? WARP_ANIMATION_ADMIN : WARP_ANIMATION_NONE);
}

void WarpMeTo(const std::vector<std::string>& arguments, Character* from)
{
	Character *victim = from->world->GetCharacter(arguments[0]);

	if (!victim || victim->nowhere)
	{
		from->ServerMsg(from->world->i18n.Format("character_not_found"));
	}
	else
	{
		if (victim->SourceAccess() < int(from->world->admin_config["cmdprotect"])
		 || victim->SourceAccess() <= from->SourceAccess())
		{
			bool bubbles = from->world->config["WarpBubbles"] && !from->IsHideWarp() && !victim->IsHideWarp();
			from->Warp(victim->mapid, victim->x, victim->y, bubbles ? WARP_ANIMATION_ADMIN : WARP_ANIMATION_NONE);
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

	if (!victim || victim->nowhere)
	{
		from->ServerMsg(from->world->i18n.Format("character_not_found"));
	}
	else
	{
		if (victim->SourceAccess() < int(from->world->admin_config["cmdprotect"])
		 || victim->SourceAccess() <= from->SourceAccess())
		{
			bool bubbles = from->world->config["WarpBubbles"] && !from->IsHideWarp() && !victim->IsHideWarp();
			victim->Warp(from->mapid, from->x, from->y, bubbles ? WARP_ANIMATION_ADMIN : WARP_ANIMATION_NONE);
		}
		else
		{
			from->ServerMsg(from->world->i18n.Format("command_access_denied"));
		}
	}
}

COMMAND_HANDLER_REGISTER(warp)
	RegisterCharacter({"warp", {"map", "x", "y"}}, Warp, CMD_FLAG_DUTY_RESTRICT);
	RegisterCharacter({"warpmeto", {"victim"}}, WarpMeTo, CMD_FLAG_DUTY_RESTRICT);
	RegisterCharacter({"warptome", {"victim"}}, WarpToMe, CMD_FLAG_DUTY_RESTRICT);

	RegisterAlias("w", "warp");
COMMAND_HANDLER_REGISTER_END(warp)

}
