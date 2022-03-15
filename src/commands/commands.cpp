/* commands/commands.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "commands.hpp"

#include "../command_source.hpp"
#include "../config.hpp"
#include "../i18n.hpp"
#include "../world.hpp"

#include "../console.hpp"
#include "../util.hpp"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace Commands
{

void command_handler_register::Register(command_handler handler, int flags)
{
	if (handler.info.partial_min_chars < 1)
		handler.info.partial_min_chars = 1;
	else if (handler.info.partial_min_chars > handler.info.name.length())
		handler.info.partial_min_chars = handler.info.name.length();

	if (flags & CMD_FLAG_DUTY_RESTRICT)
		handler.info.duty_restrict = true;

	auto result = handlers.insert(std::make_pair(handler.info.name, handler));

	if (!result.second)
		Console::Wrn("Duplicate command name: %s", handler.info.name.c_str());
}

void command_handler_register::RegisterAlias(const std::string& alias, const std::string& command)
{
	auto result = aliases.insert(std::make_pair(alias, command));

	if (!result.second)
		Console::Wrn("Duplicate command alias name: %s", alias.c_str());
}

bool command_handler_register::Handle(std::string command, const std::vector<std::string>& arguments, Command_Source* from, int alias_depth) const
{
	if (command.empty())
	{
		from->ServerMsg(from->SourceWorld()->i18n.Format("unknown_command"));
		return false;
	}

	if (alias_depth > 100)
	{
		Console::Wrn("Command alias nested too deeply or infinitely recursive: %s", command.c_str());
		return false;
	}

	auto command_result = handlers.find(command);

	if (command_result != handlers.end() && command_result->second.info.arguments.size() <= arguments.size())
	{
		AdminLevel admin_req = ADMIN_HGM;
		auto admin_result = from->SourceWorld()->admin_config.find(command);

		if (admin_result != from->SourceWorld()->admin_config.end())
			admin_req = AdminLevel(int(admin_result->second));

		int access = command_result->second.info.duty_restrict ? from->SourceDutyAccess() : from->SourceAccess();

		if (access < admin_req || (command_result->second.info.require_character && !from->SourceCharacter()))
		{
			from->ServerMsg(from->SourceWorld()->i18n.Format("unknown_command"));
			return false;
		}

		command_result->second.f(arguments, from);
		return true;
	}
	else
	{
		auto alias_result = aliases.find(command);

		if (alias_result != aliases.end())
		{
			return this->Handle(alias_result->second, arguments, from, alias_depth + 1);
		}

		auto match = handlers.end();

		UTIL_CIFOREACH(handlers, handler)
		{
			if (command.length() < handler->second.info.partial_min_chars
			 || command.length() >= handler->second.info.name.length())
				continue;

			std::string prefix = handler->second.info.name.substr(0, std::max(command.length(), handler->second.info.partial_min_chars));

			if (command.substr(0, prefix.length()) == prefix)
			{
				AdminLevel admin_req = ADMIN_HGM;
				auto admin_result = from->SourceWorld()->admin_config.find(handler->second.info.name);

				if (admin_result != from->SourceWorld()->admin_config.end())
					admin_req = AdminLevel(int(admin_result->second));

				int access = handler->second.info.duty_restrict ? from->SourceDutyAccess() : from->SourceAccess();

				if (access < admin_req || (handler->second.info.require_character && !from->SourceCharacter()))
					continue;

				// Ambiguous abbreviation
				if (match != handlers.end())
				{
					from->ServerMsg(from->SourceWorld()->i18n.Format("unknown_command"));
					return false;
				}

				match = handler;
			}
		}

		if (match != handlers.end())
		{
			if (match->second.info.arguments.size() > arguments.size())
			{
				from->ServerMsg(from->SourceWorld()->i18n.Format("command_not_enough_arguments"));
				return false;
			}

			match->second.f(arguments, from);
			return true;
		}
	}

	from->ServerMsg(from->SourceWorld()->i18n.Format("unknown_command"));
	return false;
}


command_handler_register *command_handler_register_instance;

void command_handler_register_init::init() const
{
	command_handler_register_instance = new command_handler_register();
}

command_handler_register_init::~command_handler_register_init()
{
	if (master)
		delete command_handler_register_instance;
}

static command_handler_register_init command_handler_register_init_instance(true);


}
