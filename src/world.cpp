
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "world.hpp"

#include <map>
#include <string>
#include <cmath>
#include <ctime>

#include "character.hpp"
#include "guild.hpp"
#include "party.hpp"
#include "map.hpp"

#include "eoserver.hpp"
#include "eoconst.hpp"
#include "timer.hpp"
#include "util.hpp"
#include "database.hpp"
#include "eodata.hpp"
#include "config.hpp"
#include "hash.hpp"

void world_spawn_npcs(void *world_void)
{
	World *world = static_cast<World *>(world_void);

	double spawnrate = static_cast<double>(world->config["SpawnRate"]) / 100.0;
	double current_time = Timer::GetTime();
	UTIL_VECTOR_FOREACH_ALL(world->maps, Map *, map)
	{
		UTIL_VECTOR_FOREACH_ALL(map->npcs, NPC *, npc)
		{
			if (!npc->alive && npc->dead_since + (double(npc->spawn_time) * spawnrate) < current_time)
			{
#ifdef DEBUG
				std::printf("Spawning NPC %i on map %i\n", npc->id, map->id);
#endif // DEBUG
				npc->Spawn();
			}
		}
	}
}

void world_act_npcs(void *world_void)
{
	World *world = static_cast<World *>(world_void);

	double current_time = Timer::GetTime();
	UTIL_VECTOR_FOREACH_ALL(world->maps, Map *, map)
	{
		UTIL_VECTOR_FOREACH_ALL(map->npcs, NPC *, npc)
		{
			if (npc->alive && npc->last_act + npc->act_speed < current_time)
			{
				npc->Act();
			}
		}
	}
}

void world_recover(void *world_void)
{
	World *world = static_cast<World *>(world_void);

	PacketBuilder builder(PACKET_RECOVER, PACKET_PLAYER);

	UTIL_VECTOR_FOREACH_ALL(world->characters, Character *, character)
	{
		bool updated = false;

		if (character->hp < character->maxhp)
		{
			character->hp += character->maxhp / 10;
			character->hp = std::min(character->hp, character->maxhp);
			updated = true;

			if (character->party)
			{
				character->party->UpdateHP(character);
			}
		}

		if (character->tp < character->maxtp)
		{
			character->tp += character->maxtp / 10;
			character->tp = std::min(character->tp, character->maxtp);
			updated = true;
		}

		if (updated)
		{
			builder.Reset();
			builder.AddShort(character->hp);
			builder.AddShort(character->tp);
			builder.AddShort(0); // ?
			character->player->client->SendBuilder(builder);
		}
	}
}

World::World(util::array<std::string, 5> dbinfo, const Config &eoserv_config, const Config &admin_config)
{
	if (int(this->timer.resolution * 1000.0) > 1)
	{
		std::printf("Timers set at approx. %i ms resolution\n", int(this->timer.resolution * 1000.0));
	}
	else
	{
		std::puts("Timers set at < 1 ms resolution");
	}

	this->config = eoserv_config;
	this->admin_config = admin_config;

	Database::Engine engine;
	if (dbinfo[0].compare("sqlite") == 0)
	{
		engine = Database::SQLite;
	}
	else
	{
		engine = Database::MySQL;
	}
	this->db.Connect(engine, dbinfo[1], dbinfo[2], dbinfo[3], dbinfo[4]);

	try
	{
		this->drops_config.Read(static_cast<std::string>(this->config["DropsFile"]));
	}
	catch (std::runtime_error)
	{
		std::fprintf(stderr, "WARNING: Could not load %s\n", static_cast<std::string>(this->config["DropsFile"]).c_str());
	}

	try
	{
		this->shops_config.Read(static_cast<std::string>(this->config["ShopsFile"]));
	}
	catch (std::runtime_error)
	{
		std::fprintf(stderr, "WARNING: Could not load %s\n", static_cast<std::string>(this->config["ShopsFile"]).c_str());
	}

	this->eif = new EIF(static_cast<std::string>(this->config["EIF"]));
	this->enf = new ENF(static_cast<std::string>(this->config["ENF"]));
	this->esf = new ESF(static_cast<std::string>(this->config["ESF"]));
	this->ecf = new ECF(static_cast<std::string>(this->config["ECF"]));

	this->maps.resize(static_cast<int>(this->config["Maps"])+1);
	this->maps[0] = new Map(1, this); // Just in case
	int loaded = 0;
	int npcs = 0;
	for (int i = 1; i <= static_cast<int>(this->config["Maps"]); ++i)
	{
		this->maps[i] = new Map(i, this);
		if (this->maps[i]->exists)
		{
			npcs += this->maps[i]->npcs.size();
			++loaded;
		}
	}
	std::printf("%i/%i maps loaded.\n", loaded, this->maps.size()-1);
	std::printf("%i NPCs loaded.\n", npcs);

	this->last_character_id = 0;

	this->timer.Register(new TimeEvent(world_spawn_npcs, this, 1.0, Timer::FOREVER, true));
	this->timer.Register(new TimeEvent(world_act_npcs, this, 0.05, Timer::FOREVER, true));
	this->timer.Register(new TimeEvent(world_recover, this, 90.0, Timer::FOREVER, true));

	exp_table[0] = 0;
	for (std::size_t i = 1; i < sizeof(this->exp_table)/sizeof(int); ++i)
	{
		exp_table[i] = int(util::round(std::pow(double(i), 3.0) * 133.1));
	}
}

int World::GenerateCharacterID()
{
	return ++this->last_character_id;
}

int World::GeneratePlayerID()
{
	unsigned int lowest_free_id = 1;
	restart_loop:
	UTIL_LIST_FOREACH_ALL(this->server->clients, EOClient *, client)
	{
		if (client->id == lowest_free_id)
		{
			lowest_free_id = client->id + 1;
			goto restart_loop;
		}
	}
	return lowest_free_id;
}

void World::Login(Character *character)
{
	this->characters.push_back(character);
	this->maps[character->mapid]->Enter(character);
}

void World::Logout(Character *character)
{
	this->maps[character->mapid]->Leave(character);
	UTIL_VECTOR_IFOREACH(this->characters.begin(), this->characters.end(), Character *, checkcharacter)
	{
		if (*checkcharacter == character)
		{
			this->characters.erase(checkcharacter);
			break;
		}
	}
}

void World::Msg(Character *from, std::string message)
{
	PacketBuilder builder;

	builder.SetID(PACKET_TALK, PACKET_MSG);
	if (from)
	{
		builder.AddBreakString(from->name);
	}
	else
	{
		builder.AddBreakString("Server");
	}
	builder.AddBreakString(message);

	UTIL_VECTOR_FOREACH_ALL(this->characters, Character *, character)
	{
		if (character == from)
		{
			continue;
		}

		character->player->client->SendBuilder(builder);
	}
}

void World::AdminMsg(Character *from, std::string message, int minlevel)
{
	PacketBuilder builder;

	builder.SetID(PACKET_TALK, PACKET_ADMIN);
	if (from)
	{
		builder.AddBreakString(from->name);
	}
	else
	{
		builder.AddBreakString("Server");
	}
	builder.AddBreakString(message);

	UTIL_VECTOR_FOREACH_ALL(this->characters, Character *, character)
	{
		if (character == from || character->admin < minlevel)
		{
			continue;
		}

		character->player->client->SendBuilder(builder);
	}
}

void World::AnnounceMsg(Character *from, std::string message)
{
	PacketBuilder builder;

	builder.SetID(PACKET_TALK, PACKET_ANNOUNCE);
	if (from)
	{
		builder.AddBreakString(from->name);
	}
	else
	{
		builder.AddBreakString("Server");
	}
	builder.AddBreakString(message);

	UTIL_VECTOR_FOREACH_ALL(this->characters, Character *, character)
	{
		if (character == from)
		{
			continue;
		}

		character->player->client->SendBuilder(builder);
	}
}

Character *World::GetCharacter(std::string name)
{
	Character *selected = 0;

	util::lowercase(name);

	UTIL_VECTOR_FOREACH_ALL(this->characters, Character *, character)
	{
		if (character->name.compare(name) == 0)
		{
			selected = character;
			break;
		}
	}

	return selected;
}

Character *World::GetCharacterPID(unsigned int id)
{
	Character *selected = 0;

	UTIL_VECTOR_FOREACH_ALL(this->characters, Character *, character)
	{
		if (character->player->id == id)
		{
			selected = character;
			break;
		}
	}

	return selected;
}

Character *World::GetCharacterCID(unsigned int id)
{
	Character *selected = 0;

	UTIL_VECTOR_FOREACH_ALL(this->characters, Character *, character)
	{
		if (character->id == id)
		{
			selected = character;
			break;
		}
	}

	return selected;
}

bool World::CharacterExists(std::string name)
{
	Database_Result res = this->db.Query("SELECT 1 FROM `characters` WHERE `name` = '$'", name.c_str());
	return !res.empty();
}

Character *World::CreateCharacter(Player *player, std::string name, Gender gender, int hairstyle, int haircolor, Skin race)
{
	char buffer[1024];
	std::string startmapinfo;
	std::string startmapval;
	std::string spawnmapinfo;
	std::string spawnmapval;

	if (static_cast<int>(this->config["StartMap"]))
	{
		startmapinfo = ", `map`, `x`, `y`";
		snprintf(buffer, 1024, ",%i,%i,%i", static_cast<int>(this->config["StartMap"]), static_cast<int>(this->config["StartX"]), static_cast<int>(this->config["StartY"]));
		startmapval = buffer;
	}

	if (static_cast<int>(this->config["SpawnMap"]))
	{
		spawnmapinfo = ", `spawnmap`, `spawnx`, `spawny`";
		snprintf(buffer, 1024, ",%i,%i,%i", static_cast<int>(this->config["SpawnMap"]), static_cast<int>(this->config["SpawnX"]), static_cast<int>(this->config["SpawnY"]));
		spawnmapval = buffer;
	}

	this->db.Query("INSERT INTO `characters` (`name`, `account`, `gender`, `hairstyle`, `haircolor`, `race`, `inventory`, `bank`, `paperdoll`, `spells`@@) VALUES ('$','$',#,#,#,#,'$','','$','$'@@)",
		startmapinfo.c_str(), spawnmapinfo.c_str(), name.c_str(), player->username.c_str(), gender, hairstyle, haircolor, race,
		static_cast<std::string>(this->config["StartItems"]).c_str(), static_cast<std::string>(gender?this->config["StartEquipMale"]:this->config["StartEquipFemale"]).c_str(),
		static_cast<std::string>(this->config["StartSpells"]).c_str(), startmapval.c_str(), spawnmapval.c_str());

	return new Character(name, this);
}

void World::DeleteCharacter(std::string name)
{
	this->db.Query("DELETE FROM `characters` WHERE name = '$'", name.c_str());
}

Player *World::Login(std::string username, std::string password)
{
	password = static_cast<std::string>(this->config["PasswordSalt"]) + username + password;
	sha256(password);
	Database_Result res = this->db.Query("SELECT 1 FROM `accounts` WHERE `username` = '$' AND `password` = '$'", username.c_str(), password.c_str());
	if (res.empty())
	{
		return 0;
	}
	std::map<std::string, util::variant> row = res.front();

	return new Player(username, this);
}

bool World::CreatePlayer(std::string username, std::string password, std::string fullname, std::string location, std::string email, std::string computer, std::string hdid, std::string ip)
{
	password = static_cast<std::string>(this->config["PasswordSalt"]) + username + password;
	sha256(password);
	Database_Result result = this->db.Query("INSERT INTO `accounts` (`username`, `password`, `fullname`, `location`, `email`, `computer`, `hdid`, `regip`, `created`) VALUES ('$','$','$','$','$','$','$','$',#)", username.c_str(), password.c_str(), fullname.c_str(), location.c_str(), email.c_str(), computer.c_str(), hdid.c_str(), ip.c_str(), std::time(0));
	return !result.Error();
}

bool World::PlayerExists(std::string username)
{
	Database_Result res = this->db.Query("SELECT 1 FROM `accounts` WHERE `username` = '$'", username.c_str());
	return !res.empty();
}

bool World::PlayerOnline(std::string username)
{
	if (!Player::ValidName(username))
	{
		return false;
	}

	UTIL_LIST_FOREACH_ALL(this->server->clients, EOClient *, connection)
	{
		if (connection->player)
		{
			if (connection->player->username.compare(username) == 0)
			{
				return true;
			}
		}
	}

	return false;
}

void World::Kick(Character *from, Character *victim, bool announce)
{
	victim->player->client->Close();
}

void World::Ban(Character *from, Character *victim, double duration, bool announce)
{
	this->server->AddBan(victim->player->username, victim->player->client->GetRemoteAddr(), victim->player->client->hdid, duration);
	victim->player->client->Close();
}

World::~World()
{
	delete this->eif;
	delete this->enf;
	delete this->esf;
	delete this->ecf;

	UTIL_VECTOR_FOREACH_ALL(this->maps, Map *, map)
	{
		delete map;
	}
}
