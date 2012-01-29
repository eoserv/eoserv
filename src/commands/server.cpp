
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "commands.hpp"

#include "../util.hpp"

#include "../console.hpp"
#include "../eoclient.hpp"
#include "../eoserver.hpp"
#include "../map.hpp"
#include "../player.hpp"
#include "../timer.hpp"
#include "../world.hpp"

extern bool eoserv_sig_abort;

namespace Commands
{

void ReloadMap(const std::vector<std::string>& arguments, Character* from)
{
	(void)arguments;

	if (!(from->map->Reload()))
	{
		while (!from->map->characters.empty())
		{
			from->map->characters.back()->player->client->Close();
			from->map->characters.pop_back();
		}
	}
}

void ReloadPub(const std::vector<std::string>& arguments, Command_Source* from)
{
	(void)arguments;

	Console::Out("Pub files reloaded by %s", from->SourceName().c_str());
	from->SourceWorld()->ReloadPub();
}

void ReloadConfig(const std::vector<std::string>& arguments, Command_Source* from)
{
	(void)arguments;

	Console::Out("Config reloaded by %s", from->SourceName().c_str());
	from->SourceWorld()->Rehash();
}

void ReloadQuest(const std::vector<std::string>& arguments, Command_Source* from)
{
	(void)arguments;

	Console::Out("Quests reloaded by %s", from->SourceName().c_str());
	from->SourceWorld()->ReloadQuests();
}

void Shutdown(const std::vector<std::string>& arguments, Command_Source* from)
{
	(void)arguments;

	Console::Wrn("Server shut down by %s", from->SourceName().c_str());
	eoserv_sig_abort = true;
}

void Uptime(const std::vector<std::string>& arguments, Command_Source* from)
{
	(void)arguments;

	std::string buffer = "Server started ";
	buffer += util::timeago(from->SourceWorld()->server->start, Timer::GetTime());
	from->ServerMsg(buffer);
}

COMMAND_HANDLER_REGISTER()
	using namespace std::placeholders;
	RegisterCharacter({"remap", {}, {}, 3}, ReloadMap);
	Register({"repub", {}, {}, 3}, ReloadPub);
	Register({"rehash"}, ReloadConfig);
	Register({"request", {}, {}, 3}, ReloadQuest);
	Register({"shutdown", {}, {}, 8}, Shutdown);
	Register({"uptime"}, Uptime);
COMMAND_HANDLER_REGISTER_END()

}
