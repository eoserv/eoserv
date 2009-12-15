
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "world.hpp"

#include "character.hpp"
#include "config.hpp"
#include "console.hpp"
#include "database.hpp"
#include "eoclient.hpp"
#include "eodata.hpp"
#include "eoserver.hpp"
#include "guild.hpp"
#include "hash.hpp"
#include "hook.hpp"
#include "map.hpp"
#include "npc.hpp"
#include "packet.hpp"
#include "party.hpp"
#include "player.hpp"
#include "scriptreg.hpp"
#include "util.hpp"

void world_spawn_npcs(void *world_void)
{
	World *world(static_cast<World *>(world_void));

	double spawnrate = world->config["SpawnRate"];
	double current_time = Timer::GetTime();
	UTIL_PTR_VECTOR_FOREACH(world->maps, Map, map)
	{
		UTIL_PTR_VECTOR_FOREACH(map->npcs, NPC, npc)
		{
			if (!npc->alive && npc->dead_since + (double(npc->spawn_time) * spawnrate) < current_time)
			{
#ifdef DEBUG
				Console::Dbg("Spawning NPC %i on map %i", npc->id, map->id);
#endif // DEBUG
				npc->Spawn();
			}
		}
	}
}

void world_act_npcs(void *world_void)
{
	World *world(static_cast<World *>(world_void));

	double current_time = Timer::GetTime();
	UTIL_PTR_VECTOR_FOREACH(world->maps, Map, map)
	{
		UTIL_PTR_VECTOR_FOREACH(map->npcs, NPC, npc)
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
	World *world(static_cast<World *>(world_void));

	PacketBuilder builder(PACKET_RECOVER, PACKET_PLAYER);

	UTIL_PTR_VECTOR_FOREACH(world->characters, Character, character)
	{
		bool updated = false;

		if (character->hp < character->maxhp)
		{
			character->hp += character->maxhp / 10;
			character->hp = std::min(character->hp, character->maxhp);
			updated = true;

			if (character->party)
			{
				character->party->UpdateHP(*character);
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

void world_despawn_items(void *world_void)
{
	World *world = static_cast<World *>(world_void);

	UTIL_PTR_VECTOR_FOREACH(world->maps, Map, map)
	{
		UTIL_PTR_LIST_FOREACH(map->items, Map_Item, item)
		{
			if (item->unprotecttime < (Timer::GetTime() - static_cast<double>(world->config["ItemDespawnRate"])))
			{
				map->DelItem(item->uid, 0);
			}
		}
	}
}

World::World(util::array<std::string, 5> dbinfo, const Config &eoserv_config, const Config &admin_config)
{
	if (int(this->timer.resolution * 1000.0) > 1)
	{
		Console::Out("Timers set at approx. %i ms resolution", int(this->timer.resolution * 1000.0));
	}
	else
	{
		Console::Out("Timers set at < 1 ms resolution");
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
		Console::Err("Could not load %s", static_cast<std::string>(this->config["DropsFile"]).c_str());
	}

	try
	{
		this->shops_config.Read(static_cast<std::string>(this->config["ShopsFile"]));
	}
	catch (std::runtime_error)
	{
		Console::Err("Could not load %s", static_cast<std::string>(this->config["ShopsFile"]).c_str());
	}

	try
	{
		this->arenas_config.Read(static_cast<std::string>(this->config["ArenasFile"]));
	}
	catch (std::runtime_error)
	{
		Console::Err("Could not load %s", static_cast<std::string>(this->config["ArenasFile"]).c_str());
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
	Console::Out("%i/%i maps loaded.", loaded, this->maps.size()-1);
	Console::Out("%i NPCs loaded.", npcs);

	this->last_character_id = 0;

	this->timer.Register(new TimeEvent(world_spawn_npcs, this, 1.0, Timer::FOREVER, true));
	this->timer.Register(new TimeEvent(world_act_npcs, this, 0.05, Timer::FOREVER, true));
	this->timer.Register(new TimeEvent(world_recover, this, 90.0, Timer::FOREVER, true));

	if (this->config["ItemDespawn"])
	{
		this->timer.Register(new TimeEvent(world_despawn_items, this, static_cast<double>(this->config["ItemDespawnCheck"]), Timer::FOREVER, true));
	}

	exp_table[0] = 0;
	for (std::size_t i = 1; i < sizeof(this->exp_table)/sizeof(int); ++i)
	{
		exp_table[i] = int(util::round(std::pow(double(i), 3.0) * 133.1));
	}

	for (std::size_t i = 0; i < this->boards.size(); ++i)
	{
		this->boards[i] = new Board;
	}

	this->hookmanager = new HookManager(this->config["ScriptDir"]);

	script_register(*this); // See scriptreg.cpp

	FILE *fh = fopen(static_cast<std::string>(this->config["ScriptsFile"]).c_str(), "rt");

	if (!fh)
	{
		Console::Wrn("Failed to open %s, no scripts will be loaded", static_cast<std::string>(this->config["ScriptsFile"]).c_str());
	}

	char buf[4096];

	while (fgets(buf, 4096, fh))
	{
		std::string sbuf(buf);
		sbuf = util::trim(sbuf);

		if (sbuf.length() == 0 || sbuf[0] == '#')
		{
			continue;
		}

		this->hookmanager->InitCall(sbuf.c_str());
	}

	fclose(fh);
}

int World::GenerateCharacterID()
{
	return ++this->last_character_id;
}

int World::GeneratePlayerID()
{
	unsigned int lowest_free_id = 1;
	restart_loop:
	UTIL_PTR_LIST_FOREACH(this->server->clients, EOClient, client)
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

	if (this->GetMap(character->mapid)->relog_x || this->GetMap(character->mapid)->relog_y)
	{
		character->x = this->GetMap(character->mapid)->relog_x;
		character->y = this->GetMap(character->mapid)->relog_y;
	}

	this->GetMap(character->mapid)->Enter(character);
}

void World::Logout(Character *character)
{
	if (this->GetMap(character->mapid)->exists)
	{
		this->GetMap(character->mapid)->Leave(character);

		erase_first(this->characters, character);
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

	UTIL_PTR_VECTOR_FOREACH(this->characters, Character, character)
	{
		if (*character == from)
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

	UTIL_PTR_VECTOR_FOREACH(this->characters, Character, character)
	{
		if (*character == from || character->admin < minlevel)
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

	UTIL_PTR_VECTOR_FOREACH(this->characters, Character, character)
	{
		if (*character == from)
		{
			continue;
		}

		character->player->client->SendBuilder(builder);
	}
}

void World::ServerMsg(std::string message)
{
	PacketBuilder builder;

	builder.SetID(PACKET_TALK, PACKET_SERVER);
	builder.AddString(message);

	UTIL_PTR_VECTOR_FOREACH(this->characters, Character, character)
	{
		character->player->client->SendBuilder(builder);
	}
}

Character *World::GetCharacter(std::string name)
{
	name = util::lowercase(name);

	UTIL_PTR_VECTOR_FOREACH(this->characters, Character, character)
	{
		if (character->name.compare(name) == 0)
		{
			return *character;
		}
	}

	return 0;
}

Character *World::GetCharacterPID(unsigned int id)
{
	UTIL_PTR_VECTOR_FOREACH(this->characters, Character, character)
	{
		if (character->player->id == id)
		{
			return *character;
		}
	}

	return 0;
}

Character *World::GetCharacterCID(unsigned int id)
{
	UTIL_PTR_VECTOR_FOREACH(this->characters, Character, character)
	{
		if (character->id == id)
		{
			return *character;
		}
	}

	return 0;
}

Map *World::GetMap(short id)
{
	try
	{
		return this->maps.at(id);
	}
	catch (...)
	{
		return this->maps.at(0);
	}
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

	UTIL_PTR_LIST_FOREACH(this->server->clients, EOClient, connection)
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
	if (announce)
	{
		std::string msg("Attention!! ");
		msg += victim->name + " has been removed from the game ";
		if (from) msg += "-" + from->name + " ";
		msg += "[kicked]";
		this->ServerMsg(msg);
	}

	victim->player->client->Close();
}

void World::Jail(Character *from, Character *victim, bool announce)
{
	if (announce)
	{
		std::string msg("Attention!! ");
		msg += victim->name + " has been removed from the game ";
		if (from) msg += "-" + from->name + " ";
		msg += "[jailed]";
		this->ServerMsg(msg);
	}

	victim->Warp(static_cast<int>(this->server->world->config["JailMap"]), static_cast<int>(this->server->world->config["JailX"]), static_cast<int>(this->server->world->config["JailY"]), WARP_ANIMATION_ADMIN);
}

void World::Ban(Character *from, Character *victim, int duration, bool announce)
{
	if (announce)
	{
		std::string msg("Attention!! ");
		msg += victim->name + " has been removed from the game ";
		if (from) msg += "-" + from->name + " ";
		msg += "[banned]";
		this->ServerMsg(msg);
	}

	std::string query("INSERT INTO bans (username, ip, hdid, expires, setter) VALUES ");

	query += "('" + db.Escape(victim->player->username) + "', ";
	query += util::to_string(static_cast<int>(victim->player->client->GetRemoteAddr())) + ", ";
	query += util::to_string(victim->player->client->hdid) + ", ";
	if (duration == -1)
	{
		query += "0";
	}
	else
	{
		query += util::to_string(int(std::time(0) + duration));
	}
	if (from)
	{
		query += ", '" + db.Escape(from->name) + "')";
	}
	else
	{
		query += ")";
	}

	db.Query(query.c_str(), std::time(0));

	victim->player->client->Close();
}

int World::CheckBan(const std::string *username, const IPAddress *address, const int *hdid)
{
	std::string query("SELECT COALESCE(MAX(expires),-1) AS expires FROM bans WHERE (");

	if (!username && !address && !hdid)
	{
		return -1;
	}

	if (username)
	{
		query += "username = '";
		query += db.Escape(*username);
		query += "' OR ";
	}

	if (address)
	{
		query += "ip = ";
		query += util::to_string(static_cast<int>(*const_cast<IPAddress *>(address)));
		query += " OR ";
	}

	if (hdid)
	{
		query += "hdid = ";
		query += util::to_string(*hdid);
		query += " OR ";
	}

	Database_Result res = db.Query((query.substr(0, query.length()-4) + ") AND (expires > # OR expires = 0)").c_str(), std::time(0));

	return static_cast<int>(res[0]["expires"]);
}

static std::list<int> PKExceptUnserialize(std::string serialized)
{
	std::list<int> list;
	std::size_t p = 0;
	std::size_t lastp = std::numeric_limits<std::size_t>::max();

	if (!serialized.empty() && *(serialized.end()-1) != ',')
	{
		serialized.push_back(',');
	}

	while ((p = serialized.find_first_of(',', p+1)) != std::string::npos)
	{
		list.push_back(util::to_int(serialized.substr(lastp+1, p-lastp-1)));
		lastp = p;
	}

	return list;
}

bool World::PKExcept(const Map *map)
{
	return this->PKExcept(map->id);
}

bool World::PKExcept(int mapid)
{
	if (mapid == static_cast<int>(this->config["JailMap"]))
	{
		return true;
	}

	if (this->GetMap(mapid)->arena)
	{
		return true;
	}

	std::list<int> except_list = PKExceptUnserialize(this->config["PKExcept"]);

	return std::find(except_list.begin(), except_list.end(), mapid) != except_list.end();
}

