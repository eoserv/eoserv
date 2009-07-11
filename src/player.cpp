
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "player.hpp"

#include <cstdio>

#include "hash.hpp"
#include "console.hpp"

Player::Player(std::string username, World *world)
{
	this->world = world;

	Database_Result res = this->world->db.Query("SELECT `username`, `password` FROM `accounts` WHERE `username` = '$'", username.c_str());
	if (res.empty())
	{
		return;
	}
	std::map<std::string, util::variant> row = res.front();

	this->login_time = std::time(0);

	this->online = true;
	this->character = 0;

	this->username = static_cast<std::string>(row["username"]);
	this->password = static_cast<std::string>(row["password"]);

	res = this->world->db.Query("SELECT `name` FROM `characters` WHERE `account` = '$'", username.c_str());

	typedef std::map<std::string, util::variant> Database_Row;
	UTIL_VECTOR_FOREACH_ALL(res, Database_Row, row)
	{
		Character *newchar = new Character(row["name"], world);
		this->characters.push_back(newchar);
		newchar->player = this;
	}

	this->client = 0;
}

bool Player::ValidName(std::string username)
{
	if (username.length() < 4)
	{
		return false;
	}

	if (username.length() > 12)
	{
		return false;
	}

	for (std::size_t i = 0; i < username.length(); ++i)
	{
		if (!((username[i] >= 'a' && username[i] <= 'z') || username[i] == ' ' || (username[i] >= '0' && username[i] <= '9')))
		{
			return false;
		}
	}

	return true;
}

bool Player::AddCharacter(std::string name, Gender gender, int hairstyle, int haircolor, Skin race)
{
	if (this->characters.size() > 3)
	{
		return false;
	}

	Character *newchar = this->world->CreateCharacter(this, name, gender, hairstyle, haircolor, race);

	if (!newchar)
	{
		return false;
	}

	newchar->player = this;

	this->characters.push_back(newchar);
	return true;
}

void Player::ChangePass(std::string password)
{
	password = static_cast<std::string>(this->world->config["PasswordSalt"]) + username + password;
	sha256(password);
	this->password = password;
	this->world->db.Query("UPDATE `accounts` SET `password` = '$' WHERE username = '$'", password.c_str(), this->username.c_str());
}

Player::~Player()
{
	if (this->client)
	{
#ifdef DEBUG
		Console::Dbg("Saving player '%s' (session lasted %i minutes)", this->username.c_str(), int(std::time(0) - this->login_time) / 60);
#endif // DEBUG
		this->world->db.Query("UPDATE `accounts` SET `lastused` = #, `lastip` = '$' WHERE username = '$'", std::time(0), static_cast<std::string>(this->client->GetRemoteAddr()).c_str(), this->username.c_str());

		// Disconnect the client to make sure this null pointer is never dereferenced
		this->client->Close();
		this->client->player = 0;
	}

	if (this->character)
	{
		delete this->character;
	}
}
