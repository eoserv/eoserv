
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include <csignal>
#include <ctime>

#include "../arena.hpp"
#include "../character.hpp"
#include "../console.hpp"
#include "../eoclient.hpp"
#include "../eodata.hpp"
#include "../map.hpp"
#include "../npc.hpp"
#include "../party.hpp"
#include "../player.hpp"
#include "../world.hpp"
#include "../commands/commands.hpp"

extern volatile std::sig_atomic_t eoserv_sig_abort;

static void limit_message(std::string &message, std::size_t chatlength)
{
	if (message.length() > chatlength)
	{
		message = message.substr(0, chatlength - 6) + " [...]";
	}
}

namespace Handlers
{

// Guild chat message
void Talk_Request(Character *character, PacketReader &reader)
{
	if (!character->guild) return;
	if (character->muted_until > time(0)) return;

	std::string message = reader.GetEndString(); // message
	limit_message(message, static_cast<int>(character->world->config["ChatLength"]));

	character->guild->Msg(character, message, false);
}

// Party chat messagea
void Talk_Open(Character *character, PacketReader &reader)
{
	if (!character->party) return;
	if (character->muted_until > time(0)) return;

	std::string message = reader.GetEndString(); // message
	limit_message(message, static_cast<int>(character->world->config["ChatLength"]));

	character->party->Msg(character, message, false);
}

// Global chat message
void Talk_Msg(Character *character, PacketReader &reader)
{
	if (character->muted_until > time(0)) return;

	if (character->mapid == static_cast<int>(character->world->config["JailMap"]))
	{
		return;
	}

	std::string message = reader.GetEndString();
	limit_message(message, static_cast<int>(character->world->config["ChatLength"]));

	character->world->Msg(character, message, false);
}

// Private chat message
void Talk_Tell(Character *character, PacketReader &reader)
{
	if (character->muted_until > time(0)) return;

	std::string name = reader.GetBreakString();
	std::string message = reader.GetEndString();
	limit_message(message, static_cast<int>(character->world->config["ChatLength"]));
	Character *to = character->world->GetCharacter(name);

	if (to && !to->IsHideOnline())
	{
		if (to->whispers)
		{
			to->Msg(character, message);
		}
		else
		{
			character->Msg(to, character->world->i18n.Format("whisper_blocked", to->name));
		}
	}
	else
	{
		PacketBuilder reply(PACKET_TALK, PACKET_REPLY, 2 + name.length());
		reply.AddShort(TALK_NOTFOUND);
		reply.AddString(name);
		character->Send(reply);
	}
}

// Public chat message
void Talk_Report(Character *character, PacketReader &reader)
{
	if (character->muted_until > time(0)) return;

	std::string message = reader.GetEndString();
	limit_message(message, static_cast<int>(character->world->config["ChatLength"]));

	if (message.empty())
	{
		return;
	}

	if (character->admin && message[0] == '$')
	{
		std::string command;
		std::vector<std::string> arguments = util::explode(' ', message);
		command = arguments.front().substr(1);
		arguments.erase(arguments.begin());

		character->world->Command(command, arguments, character);
	}
	else
	{
		character->map->Msg(character, message, false);
	}
}

// Admin chat message
void Talk_Admin(Character *character, PacketReader &reader)
{
	if (character->admin < ADMIN_GUARDIAN) return;
	if (character->muted_until > time(0)) return;

	std::string message = reader.GetEndString(); // message
	limit_message(message, static_cast<int>(character->world->config["ChatLength"]));

	character->world->AdminMsg(character, message, ADMIN_GUARDIAN, false);
}

// Announcement message
void Talk_Announce(Character *character, PacketReader &reader)
{
	if (character->admin < ADMIN_GUARDIAN) return;
	if (character->muted_until > time(0)) return;

	std::string message = reader.GetEndString(); // message
	limit_message(message, static_cast<int>(character->world->config["ChatLength"]));

	character->world->AnnounceMsg(character, message, false);
}

PACKET_HANDLER_REGISTER(PACKET_TALK)
	Register(PACKET_REQUEST, Talk_Request, Playing);
	Register(PACKET_OPEN, Talk_Open, Playing);
	Register(PACKET_MSG, Talk_Msg, Playing);
	Register(PACKET_TELL, Talk_Tell, Playing);
	Register(PACKET_REPORT, Talk_Report, Playing);
	Register(PACKET_ADMIN, Talk_Admin, Playing);
	Register(PACKET_ANNOUNCE, Talk_Announce, Playing);
PACKET_HANDLER_REGISTER_END(PACKET_TALK)

}
