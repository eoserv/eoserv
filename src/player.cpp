
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "player.hpp"

#include "character.hpp"
#include "console.hpp"
#include "database.hpp"
#include "eoclient.hpp"
#include "hash.hpp"
#include "world.hpp"

Player::Player(std::string username, World *world)
{
	this->world = world;

	Database_Result res = this->world->db.Query("SELECT `username`, `password` FROM `accounts` WHERE `username` = '$'", username.c_str());
	if (res.empty())
	{
		throw std::runtime_error("Player not found (" + username + ")");
	}
	std::unordered_map<std::string, util::variant> row = res.front();

	this->login_time = std::time(0);

	this->online = true;
	this->character = 0;

	this->username = static_cast<std::string>(row["username"]);
	this->password = static_cast<std::string>(row["password"]);

	res = this->world->db.Query("SELECT `name` FROM `characters` WHERE `account` = '$'", username.c_str());

	typedef std::unordered_map<std::string, util::variant> Database_Row;
	UTIL_FOREACH(res, row)
	{
		Character *newchar = new Character(row["name"], world);
		newchar->player = this;
		this->characters.push_back(newchar);
	}

	this->client = 0;
}

bool Player::ValidName(std::string username)
{
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
	if (static_cast<int>(this->characters.size()) > static_cast<int>(this->world->config["MaxCharacters"]))
	{
		return false;
	}

	Character *newchar(this->world->CreateCharacter(this, name, gender, hairstyle, haircolor, race));

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
	password = sha256(static_cast<std::string>(this->world->config["PasswordSalt"]) + username + password);
	this->password = password;
	this->world->db.Query("UPDATE `accounts` SET `password` = '$' WHERE username = '$'", password.c_str(), this->username.c_str());
}

void Player::Logout()
{
	UTIL_FOREACH(this->characters, character)
	{
		delete character;
	}
	this->characters.clear();

	if (this->client)
	{
#ifdef DEBUG
		Console::Dbg("Saving player '%s' (session lasted %i minutes)", this->username.c_str(), int(std::time(0) - this->login_time) / 60);
#endif // DEBUG
		this->world->db.Query("UPDATE `accounts` SET `lastused` = #, `hdid` = #, `lastip` = '$' WHERE username = '$'", std::time(0), this->client->hdid, static_cast<std::string>(this->client->GetRemoteAddr()).c_str(), this->username.c_str());

		// Disconnect the client to make sure this null pointer is never dereferenced
		this->client->Close();
		this->client->player = 0;
		this->client = 0; // Not reference counted!
	}
}

Player::~Player()
{
	this->Logout();
}
