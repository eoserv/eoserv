
#include "eoserv.hpp"

#include <list>
#include <map>
#include <ctime>

#include "packet.hpp"
#include "util.hpp"

Database eoserv_db;
// TODO: switch this back to a non-global object
World *the_world;

World::World(util::array<std::string, 5> dbinfo)
{
	Database::Engine engine;
	if (dbinfo[0].compare("sqlite") == 0)
	{
		engine = Database::SQLite;
	}
	else
	{
		engine = Database::MySQL;
	}
	eoserv_db.Connect(engine, dbinfo[1], dbinfo[2], dbinfo[3], dbinfo[4]);
	the_world = this;
}

void World::Login(Character *character)
{
	this->characters.push_back(character);
}

void World::Logout(Character *character)
{
	for (std::list<Character *>::iterator it = this->characters.begin(); it != this->characters.end(); ++it)
	{
		if (*it == character)
		{
			this->characters.erase(it);
			break;
		}
	}
}

void World::Msg(Character *from, std::string message)
{
	PacketBuilder builder;

	builder.SetID(PACKET_TALK, PACKET_MSG);
	builder.AddBreakString(from->name);
	builder.AddBreakString(message);

	UTIL_FOREACH(this->characters, character)
	{
		if (character == from)
		{
			continue;
		}

		character->player->client->SendBuilder(builder);
	}
}

Player::Player(std::string username)
{
	if (!Player::ValidName(username))
	{
		return;
	}
	Database_Result res = eoserv_db.Query("SELECT * FROM `accounts` WHERE `username` = '$'", username.c_str());
	if (res.empty())
	{
		return;
	}
	std::map<std::string, util::variant> row = res.front();

	this->online = true;
	this->character = 0;

	this->username = static_cast<std::string>(row["username"]);
	this->password = static_cast<std::string>(row["password"]);

	res = eoserv_db.Query("SELECT `name` FROM `characters` WHERE `account` = '$'", username.c_str());

	UTIL_FOREACH(res, row)
	{
		Character *newchar = new Character(row["name"]);
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

	for (const char *p = username.c_str(); *p != '\0'; ++p)
	{
		if (!((*p >= 'a' && *p <= 'z') || *p == ' ' || (*p >= '0' && *p < '9')))
		{
			return false;
		}
	}

	return true;
}

Player *Player::Login(std::string username, std::string password)
{
	if (!Player::ValidName(username))
	{
		return 0;
	}
	Database_Result res = eoserv_db.Query("SELECT * FROM `accounts` WHERE `username` = '$' AND `password` = '$'", username.c_str(), password.c_str());
	if (res.empty())
	{
		return 0;
	}
	std::map<std::string, util::variant> row = res.front();

	return new Player(username);
}

bool Player::Create(std::string username, std::string password, std::string fullname, std::string location, std::string email, std::string computer, std::string hdid)
{
	if (!Player::ValidName(username))
	{
		return false;
	}
	Database_Result result = eoserv_db.Query("INSERT INTO `accounts` (`username`, `password`, `fullname`, `location`, `email`, `computer`, `hdid`) VALUES ('$','$','$','$','$','$','$')", username.c_str(), password.c_str(), fullname.c_str(), location.c_str(), email.c_str(), computer.c_str(), hdid.c_str());
	return !result.Error();
}

bool Player::Exists(std::string username)
{
	if (!Player::ValidName(username))
	{
		return false;
	}
	Database_Result res = eoserv_db.Query("SELECT 1 FROM `accounts` WHERE `username` = '$'", username.c_str());
	return !res.empty();
}

bool Player::AddCharacter(std::string name, int gender, int hairstyle, int haircolor, int race)
{
	if (this->characters.size() > 3)
	{
		return false;
	}

	Character *newchar = Character::Create(this, name, gender, hairstyle, haircolor, race);

	if (!newchar)
	{
		return false;
	}

	newchar->player = this;

	this->characters.push_back(newchar);
	return true;
}

Player::~Player()
{
	if (this->client)
	{
		// Disconnect the client to make sure this null pointer is never dereferenced
		this->client->Close();
		this->client->player = 0;
	}

	if (this->character)
	{
		the_world->Logout(this->character);
	}
}

bool Character::ValidName(std::string name)
{
	if (name.length() < 4)
	{
		return false;
	}

	if (name.length() > 12)
	{
		return false;
	}

	for (const char *p = name.c_str(); *p != '\0'; ++p)
	{
		if (*p < 'a' && *p > 'z')
		{
			return false;
		}
	}

	return true;
}

Character *Character::Create(Player *player, std::string name, int gender, int hairstyle, int haircolor, int race)
{
	if (!Character::ValidName(name))
	{
		return 0;
	}
	eoserv_db.Query("INSERT INTO `characters` (`name`, `account`, `gender`, `hairstyle`, `haircolor`, `race`, `inventory`, `bank`, `paperdoll`, `spells`) VALUES ('$','$',#,#,#,#,'','','','')", name.c_str(), player->username.c_str(), gender, hairstyle, haircolor, race);
	return new Character(name);
}

Character::Character(std::string name)
{
	static unsigned int id = 1;
	if (!Character::ValidName(name))
	{
		return;
	}
	Database_Result res = eoserv_db.Query("SELECT * FROM characters WHERE name = '$'", name.c_str());
	std::map<std::string, util::variant> row = res.front();

	this->online = true;
	this->id = id++;

	this->admin = static_cast<int>(row["admin"]);
	this->name = static_cast<std::string>(row["name"]);
	this->title = static_cast<std::string>(row["title"]);

	this->clas = row["class"];
	this->gender = row["gender"];
	this->race = row["race"];
	this->hairstyle = row["hairstyle"];
	this->haircolor = row["haircolor"];

	this->map = row["map"];
	this->x = row["x"];
	this->y = row["y"];

	this->level = row["level"];
	this->exp = row["exp"];

	this->hp = row["hp"];
	this->tp = row["tp"];

	this->str = row["str"];
	this->intl = row["int"];
	this->wis = row["wis"];
	this->agi = row["agi"];
	this->con = row["con"];
	this->cha = row["cha"];
	this->statpoints = row["statpoints"];
	this->skillpoints = row["skillpoints"];

	this->weight = 0;
	this->maxweight = 250;

	this->sitting = static_cast<int>(row["sitting"]);
	this->visible = static_cast<int>(row["visible"]);

	this->player = 0;
	this->guild = 0;
	this->party = 0;
}

Character::~Character()
{
	if (this->player)
	{
		the_world->Logout(this);
	}
}
