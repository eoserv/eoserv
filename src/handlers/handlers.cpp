/* handlers/handlers.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../eoclient.hpp"
#include "../player.hpp"

#include "../console.hpp"

#include <stdexcept>

namespace Handlers
{

bool packet_handler_register::StateCheck(EOClient *client, unsigned short allow_states)
{
	switch (client->state)
	{
		case EOClient::Uninitialized:
			if (allow_states & Uninitialized)
				return true;

			break;

		case EOClient::Initialized:
			if (allow_states & Menu)
				return true;

			break;

		case EOClient::LoggedIn:
			if (client->player->character)
			{
				if (allow_states & Logging_In)
					return true;
			}
			else
			{
				if (allow_states & Character_Menu)
					return true;
			}

			break;

		case EOClient::Playing:
			if (allow_states & Playing)
				return true;

			break;
	};

	return false;
}

void packet_handler_register::Register(PacketFamily family, PacketAction action, packet_handler handler)
{
	if ((handler.allow_states & Uninitialized) && handler.fn_type != packet_handler::ClientFn)
		Console::Wrn("Uninitialized state handler accepting non-client type: %s_%s", PacketProcessor::GetFamilyName(family).c_str(), PacketProcessor::GetActionName(action).c_str());
	else if ((handler.allow_states & Menu) && handler.fn_type != packet_handler::ClientFn)
		Console::Wrn("Menu state handler accepting non-client type: %s_%s", PacketProcessor::GetFamilyName(family).c_str(), PacketProcessor::GetActionName(action).c_str());
	else if ((handler.allow_states & Character_Menu) && handler.fn_type == packet_handler::CharacterFn)
		Console::Wrn("Character_Menu state handler accepting character type: %s_%s", PacketProcessor::GetFamilyName(family).c_str(), PacketProcessor::GetActionName(action).c_str());

	if (handlers[(unsigned char)family][(unsigned char)action])
		Console::Wrn("Overriding previously registered handler: %s_%s", PacketProcessor::GetFamilyName(family).c_str(), PacketProcessor::GetActionName(action).c_str());

	handlers[(unsigned char)family][(unsigned char)action] = handler;
}

void packet_handler_register::Handle(PacketFamily family, PacketAction action, EOClient *client, PacketReader &reader, bool from_queue) const
{
	packet_handler handler;

	if (handlers[(unsigned char)family][(unsigned char)action])
	{
		handler = handlers[(unsigned char)family][(unsigned char)action];
	}
	else
	{
#ifdef DEBUG
		Console::Dbg("Unhandled packet: %s_%s (not registered)", PacketProcessor::GetFamilyName(family).c_str(), PacketProcessor::GetActionName(action).c_str());
#endif // DEBUG
		return;
	}

	if (!StateCheck(client, handler.allow_states))
	{
#ifdef DEBUG
		Console::Dbg("Unhandled packet: %s_%s (wrong client state)", PacketProcessor::GetFamilyName(family).c_str(), PacketProcessor::GetActionName(action).c_str());
#endif // DEBUG
		return;
	}

	if (!from_queue && (handler.allow_states & Playing) && !(handler.allow_states & OutOfBand))
	{
		client->queue.AddAction(reader, handler.delay);
		return;
	}

	switch (handler.fn_type)
	{
		case packet_handler::Invalid:
			break;

		case packet_handler::ClientFn:
			reinterpret_cast<client_handler_t>(handler.f)(client, reader);
			break;

		case packet_handler::PlayerFn:
			if (!client->player)
				throw std::runtime_error("Player-handled packet before login");

			reinterpret_cast<player_handler_t>(handler.f)(client->player, reader);
			break;

		case packet_handler::CharacterFn:
			if (!client->player)
				throw std::runtime_error("Character-handled packet before login");

			if (!client->player->character)
				throw std::runtime_error("Character-handled packet before character selection");

			reinterpret_cast<character_handler_t>(handler.f)(client->player->character, reader);
			break;
	}
}

void packet_handler_register::SetDelay(PacketFamily family, PacketAction action, double delay)
{
	if (handlers[(unsigned char)family][(unsigned char)action])
	{
		handlers[(unsigned char)family][(unsigned char)action].delay = delay;
	}
}

packet_handler_register *packet_handler_register_instance;

void packet_handler_register_init::init() const
{
	packet_handler_register_instance = new packet_handler_register();
}

packet_handler_register_init::~packet_handler_register_init()
{
	if (master)
		delete packet_handler_register_instance;
}

static packet_handler_register_init packet_handler_register_init_instance(true);

}
