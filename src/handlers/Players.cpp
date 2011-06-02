
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include <string>

#include "character.hpp"
#include "eoclient.hpp"
#include "world.hpp"

namespace Handlers
{

// User checking if a player is near (#find)
void Players_Accept(Character *character, PacketReader &reader)
{
	std::string name = reader.GetEndString();
	Character *victim = character->world->GetCharacter(name);

	PacketBuilder reply(PACKET_PLAYERS, PACKET_PING, name.length());

	if (victim && !victim->hidden)
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
	int online = client->server()->world->characters.size();

	UTIL_FOREACH(client->server()->world->characters, character)
	{
		if (character->hidden)
		{
			--online;
		}
	}

	PacketBuilder reply(PACKET_F_INIT, PACKET_A_INIT, 4 + client->server()->world->characters.size() * 35);
	reply.AddChar((reader.Action() == PACKET_LIST) ? INIT_FRIEND_LIST_PLAYERS : INIT_PLAYERS);
	reply.AddShort(online);
	reply.AddByte(255);
	UTIL_FOREACH(client->server()->world->characters, character)
	{
		if (character->hidden)
		{
			continue;
		}

		reply.AddBreakString(character->name);
		reply.AddBreakString(character->title);
		reply.AddChar(0); // ?
		if (character->admin >= ADMIN_HGM)
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
		else if (character->admin >= ADMIN_GUIDE)
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

	client->Send(reply);
}

PACKET_HANDLER_REGISTER(PACKET_PLAYERS)
	Register(PACKET_ACCEPT, Players_Accept, Playing);
	Register(PACKET_LIST, Players_List, Any);
	Register(PACKET_REQUEST, Players_List, Any);
PACKET_HANDLER_REGISTER_END()

}
