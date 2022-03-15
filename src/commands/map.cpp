/* commands/map.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "commands.hpp"

#include "../arena.hpp"
#include "../character.hpp"
#include "../config.hpp"
#include "../i18n.hpp"
#include "../map.hpp"
#include "../world.hpp"

#include "../util.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

namespace Commands
{

void LaunchArena(const std::vector<std::string>& arguments, Character* from)
{
	(void)arguments;

	if (from->map->arena)
		from->map->arena->Spawn(true);
}

void EvacuateMap(const std::vector<std::string>& arguments, Character* from)
{
	(void)arguments;

	from->map->Evacuate();
}

void Quake(const std::vector<std::string>& arguments, Character* from)
{
	int strength = 5;

	if (arguments.size() >= 1)
		strength = std::max(1, std::min(8, util::to_int(arguments[0])));

	from->map->Effect(MAP_EFFECT_QUAKE, strength);
}

void HideImpl(const std::vector<std::string>& arguments, Character* from, bool set)
{
	int flags = 0;

	for (const auto& flag : util::explode(",", util::lowercase(arguments[0])))
	{
		     if (flag == "invisible") flags |= Character::HideInvisible;
		else if (flag == "online")    flags |= Character::HideOnline;
		else if (flag == "npc")       flags |= Character::HideNpc;
		else if (flag == "admin")     flags |= Character::HideAdmin;
		else if (flag == "warp")      flags |= Character::HideWarp;
		else if (flag == "all")       flags |= Character::HideAll;
		else
		{
			from->ServerMsg(from->SourceWorld()->i18n.Format("invalid_hide_flag"));
			return;
		}

		int explicit_flags = (flags & ~Character::HideInvisible) << 16;
		flags |= explicit_flags;
	}

	if (set)
		from->Hide(flags);
	else
		from->Unhide(flags);
}

void Hide(const std::vector<std::string>& arguments, Character* from)
{
	if (arguments.size() >= 1)
	{
		HideImpl(arguments, from, true);
	}
	else
	{
		int flags = Character::HideAll;
		flags &= ((from->hidden & 0xFFFF0000) | ~(from->hidden >> 16));

		if (from->IsHideInvisible())
			from->Unhide(flags);
		else
			from->Hide(flags);
	}
}

void Unhide(const std::vector<std::string>& arguments, Character* from)
{
	if (arguments.size() >= 1)
	{
		HideImpl(arguments, from, false);
	}
	else
	{
		int flags = Character::HideAll;
		flags &= ((from->hidden & 0xFFFF0000) | ~(from->hidden >> 16));

		from->Unhide(flags);
	}
}

void Board(const std::vector<std::string>& arguments, Character* from)
{
	short boardid = ((arguments.size() >= 1) ? util::to_int(arguments[0]) : int(from->world->config["AdminBoard"])) - 1;

	if (boardid != int(from->world->config["AdminBoard"]) - 1
	 || from->SourceAccess() >= int(from->world->admin_config["reports"]))
	{
		if (std::size_t(boardid) < from->world->boards.size())
		{
			from->board = from->world->boards[boardid];
			from->ShowBoard();
		}
	}
}

COMMAND_HANDLER_REGISTER(map)
	RegisterCharacter({"arena"}, LaunchArena, CMD_FLAG_DUTY_RESTRICT);
	RegisterCharacter({"evacuate"}, EvacuateMap);
	RegisterCharacter({"quake", {}, {"strength"}}, Quake);
	RegisterCharacter({"hide", {}, {"flags"}}, Hide, CMD_FLAG_DUTY_RESTRICT);
	RegisterCharacter({"show", {}, {"flags"}}, Unhide, CMD_FLAG_DUTY_RESTRICT);
	RegisterCharacter({"board", {}, {"board"}}, Board);

	RegisterAlias("a", "arena");
	RegisterAlias("e", "evacuate");
	RegisterAlias("q", "quake");
	RegisterAlias("x", "hide");
	RegisterAlias("h", "hide");
	RegisterAlias("s", "show");
COMMAND_HANDLER_REGISTER_END(map)

}
