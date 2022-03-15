/* command_source.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "command_source.hpp"

#include "console.hpp"

#include <string>

System_Command_Source::System_Command_Source(World* world)
	: world(world)
{ }

AdminLevel System_Command_Source::SourceAccess() const
{
	return ADMIN_HGM;
}

AdminLevel System_Command_Source::SourceDutyAccess() const
{
	return ADMIN_HGM;
}

std::string System_Command_Source::SourceName() const
{
	return "server";
}

Character* System_Command_Source::SourceCharacter()
{
	return 0;
}

World* System_Command_Source::SourceWorld()
{
	return world;
}

void System_Command_Source::ServerMsg(std::string msg)
{
	Console::Out("%s", msg.c_str());
}

void System_Command_Source::StatusMsg(std::string msg)
{
	Console::Out("%s", msg.c_str());
}
