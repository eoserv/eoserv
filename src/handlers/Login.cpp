/* handlers/Login.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../eoclient.hpp"
#include "../eodata.hpp"
#include "../eoserver.hpp"
#include "../packet.hpp"
#include "../player.hpp"
#include "../world.hpp"
#include "../extra/seose_compat.hpp"

#include "../util.hpp"
#include "../util/secure_string.hpp"

#include <cstddef>
#include <string>
#include <utility>

namespace Handlers
{

// Log in to an account
void Login_Request(EOClient *client, PacketReader &reader)
{
	std::string username = reader.GetBreakString();
	util::secure_string password(std::move(reader.GetBreakString()));

	if (username.length() > std::size_t(int(client->server()->world->config["AccountMaxLength"]))
	 || password.str().length() > std::size_t(int(client->server()->world->config["PasswordMaxLength"])))
	{
		return;
	}

	username = util::lowercase(username);

	if (client->server()->world->config["SeoseCompat"])
		password = std::move(seose_str_hash(password.str(), client->server()->world->config["SeoseCompatKey"]));

	if (client->server()->world->CheckBan(&username, 0, 0) != -1)
	{
		PacketBuilder reply(PACKET_F_INIT, PACKET_A_INIT, 2);
		reply.AddByte(INIT_BANNED);
		reply.AddByte(INIT_BAN_PERM);
		client->Send(reply);
		client->Close();
		return;
	}

	if (username.length() < std::size_t(int(client->server()->world->config["AccountMinLength"])))
	{
		PacketBuilder reply(PACKET_LOGIN, PACKET_REPLY, 2);
		reply.AddShort(LOGIN_WRONG_USER);
		client->Send(reply);
		return;
	}

	if (password.str().length() < std::size_t(int(client->server()->world->config["PasswordMinLength"])))
	{
		PacketBuilder reply(PACKET_LOGIN, PACKET_REPLY, 2);
		reply.AddShort(LOGIN_WRONG_USERPASS);
		client->Send(reply);
		return;
	}

	if (client->server()->world->characters.size() >= static_cast<std::size_t>(static_cast<int>(client->server()->world->config["MaxPlayers"])))
	{
		PacketBuilder reply(PACKET_LOGIN, PACKET_REPLY, 2);
		reply.AddShort(LOGIN_BUSY);
		client->Send(reply);
		client->Close();
		return;
	}

	LoginReply login_reply = client->server()->world->LoginCheck(username, std::move(password));

	if (login_reply != LOGIN_OK)
	{
		PacketBuilder reply(PACKET_LOGIN, PACKET_REPLY, 2);
		reply.AddShort(login_reply);
		client->Send(reply);

		int max_login_attempts = int(client->server()->world->config["MaxLoginAttempts"]);

		if (max_login_attempts != 0 && ++client->login_attempts >= max_login_attempts)
		{
			client->Close();
		}

		return;
	}

	client->player = client->server()->world->Login(username);

	if (!client->player)
	{
		// Someone deleted the account between checking it and logging in
		PacketBuilder reply(PACKET_LOGIN, PACKET_REPLY, 2);
		reply.AddShort(LOGIN_WRONG_USER);
		client->Send(reply);
		return;
	}

	client->player->id = client->id;
	client->player->client = client;
	client->state = EOClient::LoggedIn;

	PacketBuilder reply(PACKET_LOGIN, PACKET_REPLY, 5 + client->player->characters.size() * 34);
	reply.AddShort(LOGIN_OK);
	reply.AddChar(client->player->characters.size());
	reply.AddByte(2);
	reply.AddByte(255);
	UTIL_FOREACH(client->player->characters, character)
	{
		reply.AddBreakString(character->SourceName());
		reply.AddInt(character->id);
		reply.AddChar(character->level);
		reply.AddChar(character->gender);
		reply.AddChar(character->hairstyle);
		reply.AddChar(character->haircolor);
		reply.AddChar(character->race);
		reply.AddChar(character->admin);
		character->AddPaperdollData(reply, "BAHSW");

		reply.AddByte(255);
	}
	client->Send(reply);
}

PACKET_HANDLER_REGISTER(PACKET_LOGIN)
	Register(PACKET_REQUEST, Login_Request, Menu, 1.0);
PACKET_HANDLER_REGISTER_END(PACKET_LOGIN)

}
