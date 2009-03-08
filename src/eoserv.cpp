
#include "eoserv.hpp"

#include <list>
#include <vector>
#include <ctime>
#include <cmath>

#include "packet.hpp"
#include "util.hpp"
#include "eoconst.hpp"

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

	this->maps.resize(279);
	this->maps[0] = new Map(1); // Just in case
	for (int i = 1; i <= 278; ++i)
	{
		this->maps[i] = new Map(i);
	}

	the_world = this;
}

void World::Login(Character *character)
{
	this->characters.push_back(character);
	this->maps[character->mapid]->Enter(character);
}

void World::Logout(Character *character)
{
	this->maps[character->mapid]->Leave(character);
	UTIL_FOREACH(characters, checkcharacter)
	{
		if (checkcharacter == character)
		{
			this->characters.erase(util_it);
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

Map::Map(int id)
{
	char filename[6];
	std::sprintf(filename, "%05i", std::abs(id));
	this->filename = filename;
}

void Map::Enter(Character *character)
{
	PacketBuilder builder;
	this->characters.push_back(character);
	character->map = this;

	builder.SetID(PACKET_PLAYERS, PACKET_AGREE);

	builder.AddByte(255);
	builder.AddBreakString(character->name);
	builder.AddShort(character->player->id);
	builder.AddShort(character->mapid); // Map ID
	builder.AddShort(character->x); // Map X
	builder.AddShort(character->y); // Map Y
	builder.AddChar(character->direction); // Direction
	builder.AddChar(6); // ?
	builder.AddString("SEX"); // guild tag
	builder.AddChar(character->level); // Level
	builder.AddChar(character->gender); // sex (0 = female, 1 = male)
	builder.AddChar(character->hairstyle); // hair style
	builder.AddChar(character->haircolor); // hair color
	builder.AddChar(character->race); // race (0 = white, 1 = azn, 2 = nigger, 3 = orc, 4 = skeleton, 5 = panda)
	builder.AddShort(character->maxhp); // Max HP (?)
	builder.AddShort(character->hp); // HP (?)
	builder.AddShort(character->maxtp); // Max TP (?)
	builder.AddShort(character->tp); // TP (?)
	// equipment
	builder.AddShort(0); // ??
	builder.AddShort(0); // ??
	builder.AddShort(0); // ??
	builder.AddShort(0); // shoes
	builder.AddShort(0); // armor
	builder.AddShort(0); // ??
	builder.AddShort(0); // hat
	builder.AddShort(0); // shield
	builder.AddShort(0); // weapon
	builder.AddChar(character->sitting); // standing
	builder.AddChar(0); // visible
	builder.AddByte(255);
	builder.AddChar(1); // 0 = NPC, 1 = player

	UTIL_FOREACH(characters, checkcharacter)
	{
		if (checkcharacter == character)
		{
			continue;
		}

		checkcharacter->player->client->SendBuilder(builder);
	}
}

void Map::Leave(Character *character)
{
	UTIL_FOREACH(characters, checkcharacter)
	{
		if (checkcharacter == character)
		{
			this->characters.erase(util_it);
			break;
		}
	}
	character->map = 0;
}

void Map::Msg(Character *from, std::string message)
{
	PacketBuilder builder;

	builder.SetID(PACKET_TALK, PACKET_PLAYER);
	builder.AddShort(from->player->id);
	builder.AddString(message);

	UTIL_FOREACH(characters, character)
	{
		if (character == from)
		{
			continue;
		}

		character->player->client->SendBuilder(builder);
	}
}

void Map::Walk(Character *from, int direction)
{
	PacketBuilder builder;

	switch (direction)
	{
		case DIRECTION_UP:
			--from->y;
			break;
		case DIRECTION_RIGHT:
			++from->x;
			break;
		case DIRECTION_DOWN:
			++from->y;
			break;
		case DIRECTION_LEFT:
			--from->x;
			break;
	}

	from->direction = direction;

	builder.SetID(PACKET_WALK, PACKET_PLAYER);
	builder.AddShort(from->player->id);
	builder.AddChar(direction);
	builder.AddChar(from->x);
	builder.AddChar(from->y);

	UTIL_FOREACH(characters, character)
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

bool Player::Online(std::string username)
{
	// TODO: implement this
	return false;

	if (!Player::ValidName(username))
	{
		return false;
	}
	Database_Result res = eoserv_db.Query("SELECT 1 FROM `accounts` WHERE `username` = '$' AND `online` = 1", username.c_str());
	return !res.empty();
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
		delete this->character;
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
	Database_Result res = eoserv_db.Query("SELECT * FROM `characters` WHERE `name` = '$'", name.c_str());
	std::map<std::string, util::variant> row = res.front();

	this->online = true;
	this->id = id++;

	this->admin = static_cast<int>(row["admin"]);
	this->name = static_cast<std::string>(row["name"]);
	this->title = static_cast<std::string>(row["title"]);
	this->home = static_cast<std::string>(row["home"]);
	this->partner = static_cast<std::string>(row["partner"]);

	this->clas = row["class"];
	this->gender = row["gender"];
	this->race = row["race"];
	this->hairstyle = row["hairstyle"];
	this->haircolor = row["haircolor"];

	this->mapid = row["map"];
	this->x = row["x"];
	this->y = row["y"];
	this->direction = row["direction"];

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

	this->bankmax = static_cast<int>(row["bankmax"]);
	this->goldbank = static_cast<int>(row["goldbank"]);

	this->usage = static_cast<int>(row["usage"]);

	this->player = 0;
	this->guild = 0;
	this->party = 0;
	this->map = 0;
}

Character::~Character()
{
	if (this->player)
	{
		the_world->Logout(this);
	}

#ifdef DEBUG
	std::printf("Saving character '%s'\n", this->name.c_str());
#endif // DEBUG
	eoserv_db.Query("UPDATE `characters` SET `title` = '$', `home` = '$', `partner` = '$', `class` = #, `gender` = #, `race` = #, "
	                "`hairstyle` = #, `haircolor` = #, `map` = #, `x` = #, `y` = #, `direction` = #, `level` = #, `exp` = #, `hp` = #, `tp` = #, "
	                "`str` = #, `int` = #, `wis` = #, `agi` = #, `con` = #, `cha` = #, `statpoints` = #, `skillpoints` = #, `sitting` = #, "
	                "`bankmax` = #, `goldbank` = #, `usage` = # WHERE `name` = '$'",
                    this->title.c_str(), this->home.c_str(), this->partner.c_str(), this->clas, this->gender, this->race,
	                this->hairstyle, this->haircolor, this->mapid, this->x, this->y, this->direction, this->level, this->exp, this->hp, this->tp,
	                this->str, this->intl, this->wis, this->agi, this->con, this->cha, this->statpoints, this->skillpoints, this->sitting,
	                this->bankmax, this->goldbank, this->usage, this->name.c_str());
}
