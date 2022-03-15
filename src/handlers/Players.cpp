/* handlers/Players.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../config.hpp"
#include "../eoclient.hpp"
#include "../eoserver.hpp"
#include "../packet.hpp"
#include "../player.hpp"
#include "../world.hpp"

#include "../util.hpp"

#include <string>

namespace Handlers
{

// User checking if a player is near (#find)
void Players_Accept(Character *character, PacketReader &reader)
{
	std::string name = reader.GetEndString();
	Character *victim = character->world->GetCharacter(name);

	PacketBuilder reply(PACKET_PLAYERS, PACKET_PING, name.length());

	if (victim && !victim->IsHideInvisible())
	{
		if (victim->mapid == character->mapid && !victim->nowhere)
		{
			reply.SetID(PACKET_PLAYERS, PACKET_PONG);
		}
		else
		{
			reply.SetID(PACKET_PLAYERS, PACKET_NET3);
		}
	}

	reply.AddString(name);
	character->Send(reply);
}

// Requested a list of online players
void Players_List(EOClient *client, PacketReader &reader)
{
	Player* from_player = client->player;
	Character* from_character = from_player ? from_player->character : nullptr;
	int seehide = client->server()->world->admin_config["seehide"];
	int cmdprotect = client->server()->world->admin_config["cmdprotect"];

	auto really_hidden = [&](const Character* character)
	{
		return !(from_character && (
		       from_character == character
		    || (from_character->admin >= seehide && character->admin < cmdprotect)
		    || (from_character->admin >= seehide && character->admin >= cmdprotect && from_character->admin > character->admin)
		));
	};

	if (!client->server()->world->config["SLN"] && !client->server()->world->config["AllowStats"] && !from_character)
	{
		return;
	}

	int online = client->server()->world->characters.size();

	UTIL_FOREACH(client->server()->world->characters, character)
	{
		bool hide_online = character->IsHideOnline();

		if (hide_online && really_hidden(character))
			--online;
	}

	bool is_friends_list = (reader.Action() == PACKET_LIST);
	std::string hidden_admin_suffix = client->server()->world->i18n.Format("hidden_admin_suffix");

	PacketBuilder reply(PACKET_F_INIT, PACKET_A_INIT, 4 + online * (is_friends_list ? 13 : (36 + hidden_admin_suffix.length())));
	reply.AddChar(is_friends_list ? INIT_FRIEND_LIST_PLAYERS : INIT_PLAYERS);
	reply.AddShort(online);
	reply.AddByte(255);
	UTIL_FOREACH(client->server()->world->characters, character)
	{
		bool hide_online = character->IsHideOnline();
		bool hide_admin = character->IsHideAdmin();

		std::string name_suffix;

		if (hide_online && really_hidden(character))
			continue;

		if (hide_online)
			name_suffix = " " + hidden_admin_suffix;

		reply.AddBreakString(character->SourceName() + name_suffix);

		// Full information is not sent for friends list requests
		if (!is_friends_list)
		{
			reply.AddBreakString(character->title);
			reply.AddChar(0); // ?
			if (character->bot && !from_character)
			{
				reply.AddChar(ICON_SLN_BOT);
			}
			else if (character->admin >= ADMIN_HGM && !hide_admin)
			{
				if (character->party)
				{
					reply.AddChar(ICON_HGM_PARTY);
				}
				else
				{
					reply.AddChar(ICON_HGM);
				}
			}
			else if (character->admin >= ADMIN_GUIDE && !hide_admin)
			{
				if (character->party)
				{
					reply.AddChar(ICON_GM_PARTY);
				}
				else
				{
					reply.AddChar(ICON_GM);
				}
			}
			else
			{
				if (character->party)
				{
					reply.AddChar(ICON_PARTY);
				}
				else
				{
					reply.AddChar(ICON_NORMAL);
				}
			}
			reply.AddChar(character->clas);
			reply.AddString(character->PaddedGuildTag());
			reply.AddByte(255);
		}
		else
		{
			reply.AddBreakString(character->SourceName());
		}
	}

	client->Send(reply);
}

PACKET_HANDLER_REGISTER(PACKET_PLAYERS)
	Register(PACKET_ACCEPT, Players_Accept, Playing);
	Register(PACKET_LIST, Players_List, Any);
	Register(PACKET_REQUEST, Players_List, Any);
PACKET_HANDLER_REGISTER_END(PACKET_PLAYERS)

}
