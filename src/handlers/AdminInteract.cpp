/* handlers/AdminInteract.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../config.hpp"
#include "../i18n.hpp"
#include "../world.hpp"

#include <string>

namespace Handlers
{

// "Talk to admin" message
void AdminInteract_Tell(Character *character, PacketReader &reader)
{
	std::string message = reader.GetEndString();

	if (character->world->config["OldReports"])
	{
		message = character->world->i18n.Format("admin_request", message);
		character->world->AdminMsg(character, message, static_cast<int>(character->world->admin_config["reports"]));
	}
	else
	{
		character->world->AdminRequest(character, message);
	}
}

// User report
void AdminInteract_Report(Character *character, PacketReader &reader)
{
	std::string reportee = reader.GetBreakString();
	std::string message = reader.GetEndString();

	if (character->world->config["OldReports"])
	{
		message = character->world->i18n.Format("admin_report", reportee, message);
		character->world->AdminMsg(character, message, static_cast<int>(character->world->admin_config["reports"]));
	}
	else
	{
		character->world->AdminReport(character, reportee, message);
	}
}

PACKET_HANDLER_REGISTER(PACKET_ADMININTERACT)
	Register(PACKET_TELL, AdminInteract_Tell, Playing | OutOfBand);
	Register(PACKET_REPORT, AdminInteract_Report, Playing | OutOfBand);
PACKET_HANDLER_REGISTER_END(PACKET_ADMININTERACT)

}
