
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

#include "util.hpp"

#include "character.hpp"
#include "eodata.hpp"
#include "eoserver.hpp"
#include "player.hpp"
#include "world.hpp"

CLIENT_F_FUNC(Login)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Logging in to an account
		{
			if (this->state != EOClient::Initialized) return false;

			std::string username = reader.GetBreakString();
			std::string password = reader.GetBreakString();

			username = util::lowercase(username);

			if (this->server()->world->CheckBan(&username, 0, 0) != -1)
			{
				reply.SetID(0);
				reply.AddByte(INIT_BANNED);
				reply.AddByte(INIT_BAN_PERM);
				CLIENT_SENDRAW(reply);
				this->Close();
				return false;
			}

			reply.SetID(PACKET_LOGIN, PACKET_REPLY);

			if (this->server()->world->characters.size() >= static_cast<std::size_t>(static_cast<int>(this->server()->world->config["MaxPlayers"])))
			{
				reply.AddShort(LOGIN_BUSY);
				CLIENT_SEND(reply);
				this->Close();
				return false;
			}

			LoginReply login_reply = this->server()->world->LoginCheck(username, password);

			if (login_reply != LOGIN_OK)
			{
				reply.AddShort(login_reply);
				CLIENT_SEND(reply);
				return true;
			}

			this->player = this->server()->world->Login(username);

			if (!this->player)
			{
				// Someone deleted the account between checking it and logging in
				reply.AddShort(LOGIN_WRONG_USER);
				CLIENT_SEND(reply);
				return true;
			}

			this->player->id = this->id;
			this->player->client = this; // Not reference counted!
			this->state = EOClient::LoggedIn;

			reply.AddShort(LOGIN_OK);
			reply.AddChar(this->player->characters.size());
			reply.AddByte(2);
			reply.AddByte(255);
			UTIL_PTR_VECTOR_FOREACH(this->player->characters, Character, character)
			{
				reply.AddBreakString(character->name);
				reply.AddInt(character->id);
				reply.AddChar(character->level);
				reply.AddChar(character->gender);
				reply.AddChar(character->hairstyle);
				reply.AddChar(character->haircolor);
				reply.AddChar(character->race);
				reply.AddChar(character->admin);
				reply.AddShort(this->server()->world->eif->Get(character->paperdoll[Character::Boots])->dollgraphic);
				reply.AddShort(this->server()->world->eif->Get(character->paperdoll[Character::Armor])->dollgraphic);
				reply.AddShort(this->server()->world->eif->Get(character->paperdoll[Character::Hat])->dollgraphic);
				reply.AddShort(this->server()->world->eif->Get(character->paperdoll[Character::Shield])->dollgraphic);
				reply.AddShort(this->server()->world->eif->Get(character->paperdoll[Character::Weapon])->dollgraphic);
				reply.AddByte(255);
			}
			CLIENT_SEND(reply);

		}
		break;

		default:
			return false;
	}

	return true;
}
