
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "commands.hpp"

#include <functional>

#include "../util.hpp"

#include "world.hpp"

namespace Commands
{

static void do_punishment(Command_Source* from, Character* victim, std::function<void(World*, Command_Source*, Character*, bool)> f, bool announce)
{
	World* world = from->SourceWorld();

	if (!victim)
	{
		from->ServerMsg(world->i18n.Format("character_not_found"));
	}
	else
	{
		if (victim->admin < from->SourceAccess())
		{
			f(world, from, victim, announce);
		}
		else
		{
			from->ServerMsg(world->i18n.Format("command_access_denied"));
		}
	}
}

void Kick(const std::vector<std::string>& arguments, Command_Source* from, bool announce = true)
{
	Character* victim = from->SourceWorld()->GetCharacter(arguments[0]);
	do_punishment(from, victim, std::mem_fn(&World::Kick), announce);
}

void Ban(const std::vector<std::string>& arguments, Command_Source* from, bool announce = true)
{
	Character* victim = from->SourceWorld()->GetCharacter(arguments[0]);
	int duration = -1;

	if (arguments.size() >= 2)
	{
		if (util::lowercase(arguments[1]) != "forever")
			duration = int(util::tdparse(arguments[1]));
	}
	else
	{
		duration = int(util::tdparse(from->SourceWorld()->config["DefaultBanLength"]));
	}

	do_punishment(from, victim, [duration](World* world, Command_Source* from, Character* victim, bool announce)
		{
			world->Ban(from, victim, duration, announce);
		}, announce);
}

void Jail(const std::vector<std::string>& arguments, Command_Source* from, bool announce = true)
{
	Character* victim = from->SourceWorld()->GetCharacter(arguments[0]);
	do_punishment(from, victim, std::mem_fn(&World::Kick), announce);
}

void Mute(const std::vector<std::string>& arguments, Command_Source* from, bool announce = true)
{
	Character* victim = from->SourceWorld()->GetCharacter(arguments[0]);
	do_punishment(from, victim, std::mem_fn(&World::Mute), announce);
}

COMMAND_HANDLER_REGISTER()
	using namespace std::placeholders;
	Register({"kick", {"victim"}, {}}, std::bind(Kick, _1, _2, true));
	Register({"skick", {"victim"}, {}, 2}, std::bind(Kick, _1, _2, false));
	Register({"ban", {"victim"}, {"duration"}}, std::bind(Ban, _1, _2, true));
	Register({"sban", {"victim"}, {"duration"}, 2}, std::bind(Ban, _1, _2, false));
	Register({"jail", {"victim"}, {}}, std::bind(Jail, _1, _2, false));
	Register({"sjail", {"victim"}, {}, 2}, std::bind(Jail, _1, _2, false));
	Register({"mute", {"victim"}, {}}, std::bind(Mute, _1, _2, false));
	Register({"smute", {"victim"}, {}, 2}, std::bind(Mute, _1, _2, false));

	RegisterAlias("k", "kick");
	RegisterAlias("b", "ban");
	RegisterAlias("j", "jail");
	RegisterAlias("m", "mute");
COMMAND_HANDLER_REGISTER_END()

}
