
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "commands.hpp"

#include "../util.hpp"

#include "../arena.hpp"
#include "../config.hpp"
#include "../map.hpp"
#include "../world.hpp"

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

void Hide(const std::vector<std::string>& arguments, Character* from)
{
	(void)arguments;

	if (from->hidden)
		from->Unhide();
	else
		from->Hide();
}

void Board(const std::vector<std::string>& arguments, Character* from)
{
	short boardid = ((arguments.size() >= 1) ? util::to_int(arguments[0]) : int(from->world->config["AdminBoard"])) - 1;

	if (boardid != int(from->world->config["AdminBoard"]) - 1 || from->admin >= int(from->world->admin_config["reports"]))
	{
		if (std::size_t(boardid) < from->world->boards.size())
		{
			from->board = from->world->boards[boardid];
			from->ShowBoard();
		}
	}
}

COMMAND_HANDLER_REGISTER()
	RegisterCharacter({"arena"}, LaunchArena);
	RegisterCharacter({"evacuate"}, EvacuateMap);
	RegisterCharacter({"quake", {}, {"strength"}}, Quake);
	RegisterCharacter({"hide"}, Hide);
	RegisterCharacter({"board", {}, {"board"}}, Board);

	RegisterAlias("a", "arena");
	RegisterAlias("e", "evacuate");
	RegisterAlias("h", "hide");
	RegisterAlias("q", "quake");
	RegisterAlias("x", "hide");
COMMAND_HANDLER_REGISTER_END()

}
