
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

CLIENT_F_FUNC(AdminInteract)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_TELL: // "Talk to admin" message
		{
			if (this->state < EOClient::PlayingModal) return false;

			std::string message = reader.GetEndString();

			if (this->server->world->config["OldReports"])
			{
				message = "[Request] " + message;
				this->server->world->AdminMsg(this->player->character, message, static_cast<int>(this->server->world->admin_config["reports"]));
			}
			else
			{
				this->server->world->AdminRequest(this->player->character, message);
			}
		}
		break;

		case PACKET_REPORT: // User report
		{
			if (this->state < EOClient::PlayingModal) return false;

			std::string reportee = reader.GetBreakString();
			std::string message = reader.GetEndString();

			if (this->server->world->config["OldReports"])
			{
				message = "[Report:" + reportee + "] " + message;
				this->server->world->AdminMsg(this->player->character, message, static_cast<int>(this->server->world->admin_config["reports"]));
			}
			else
			{
				this->server->world->AdminReport(this->player->character, reportee, message);
			}
		}
		break;

		default:
			return false;
	}

	return true;
}
