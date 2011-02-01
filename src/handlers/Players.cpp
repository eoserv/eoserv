
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

#include <string>

#include "character.hpp"
#include "player.hpp"
#include "world.hpp"

CLIENT_F_FUNC(Players)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_ACCEPT: // User checking if a player is near (#find)
		{
			if (this->state < EOClient::PlayingModal) return false;

			std::string name = reader.GetEndString();
			Character *victim = this->server()->world->GetCharacter(name);

			if (victim && !victim->hidden)
			{
				if (victim->mapid == this->player->character->mapid && !victim->nowhere)
				{
					reply.SetID(PACKET_PLAYERS, PACKET_NET2);
				}
				else
				{
					reply.SetID(PACKET_PLAYERS, PACKET_NET3);
				}
			}
			else
			{
				reply.SetID(PACKET_PLAYERS, PACKET_NET);
			}

			reply.AddString(name);
			CLIENT_SEND(reply);
		}

		case PACKET_LIST: // Opened friends list
		case PACKET_REQUEST: // Requested a list of online players
		{
			int online = this->server()->world->characters.size();

			UTIL_PTR_VECTOR_FOREACH(this->server()->world->characters, Character, character)
			{
				if (character->hidden)
				{
					--online;
				}
			}

			reply.SetID(0);
			reply.AddChar(action == PACKET_LIST ? INIT_FRIEND_LIST_PLAYERS : INIT_PLAYERS);
			reply.AddShort(online);
			reply.AddByte(255);
			UTIL_PTR_VECTOR_FOREACH(this->server()->world->characters, Character, character)
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
			CLIENT_SENDRAW(reply);
		}
		break;

		default:
			return false;
	}

	return true;
}
