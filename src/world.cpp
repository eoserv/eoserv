
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "world.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

#include "console.hpp"
#include "database.hpp"
#include "hash.hpp"
#include "util.hpp"

#include "character.hpp"
#include "command_source.hpp"
#include "config.hpp"
#include "eoclient.hpp"
#include "eodata.hpp"
#include "eoserver.hpp"
#include "guild.hpp"
#include "map.hpp"
#include "npc.hpp"
#include "packet.hpp"
#include "party.hpp"
#include "player.hpp"
#include "quest.hpp"
#include "commands/commands.hpp"

void world_spawn_npcs(void *world_void)
{
	World *world(static_cast<World *>(world_void));

	double spawnrate = world->config["SpawnRate"];
	double current_time = Timer::GetTime();
	UTIL_FOREACH(world->maps, map)
	{
		UTIL_FOREACH(map->npcs, npc)
		{
			if ((!npc->alive && npc->dead_since + (double(npc->spawn_time) * spawnrate) < current_time)
			 && (!npc->Data()->child || (npc->parent && npc->parent->alive && world->config["RespawnBossChildren"])))
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
	UTIL_FOREACH(world->maps, map)
	{
		UTIL_FOREACH(map->npcs, npc)
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

	UTIL_FOREACH(world->characters, character)
	{
		bool updated = false;

		if (character->hp < character->maxhp)
		{
			if (character->sitting != SIT_STAND) character->hp += character->maxhp * double(world->config["SitHPRecoverRate"]);
			else                                 character->hp += character->maxhp * double(world->config["HPRecoverRate"]);

			character->hp = std::min(character->hp, character->maxhp);
			updated = true;

			if (character->party)
			{
				character->party->UpdateHP(character);
			}
		}

		if (character->tp < character->maxtp)
		{
			if (character->sitting != SIT_STAND) character->tp += character->maxtp * double(world->config["SitTPRecoverRate"]);
			else                                 character->tp += character->maxtp * double(world->config["TPRecoverRate"]);

			character->tp = std::min(character->tp, character->maxtp);
			updated = true;
		}

		if (updated)
		{
			PacketBuilder builder(PACKET_RECOVER, PACKET_PLAYER, 6);
			builder.AddShort(character->hp);
			builder.AddShort(character->tp);
			builder.AddShort(0); // ?
			character->Send(builder);
		}
	}
}

void world_npc_recover(void *world_void)
{
	World *world(static_cast<World *>(world_void));

	UTIL_FOREACH(world->maps, map)
	{
		UTIL_FOREACH(map->npcs, npc)
		{
			if (npc->alive && npc->hp < npc->Data()->hp)
			{
				npc->hp += npc->Data()->hp * double(world->config["NPCRecoverRate"]);

				npc->hp = std::min(npc->hp, npc->Data()->hp);
			}
		}
	}
}

void world_warp_suck(void *world_void)
{
	struct Warp_Suck_Action
	{
		Character *character;
		short map;
		unsigned char x;
		unsigned char y;

		Warp_Suck_Action(Character *character, short map, unsigned char x, unsigned char y)
			: character(character)
			, map(map)
			, x(x)
			, y(y)
		{ }
	};

	std::vector<Warp_Suck_Action> actions;

	World *world(static_cast<World *>(world_void));

	double now = Timer::GetTime();
	double delay = world->config["WarpSuck"];
	Map_Warp *warp = 0;

	UTIL_FOREACH(world->maps, map)
	{
		UTIL_FOREACH(map->characters, character)
		{
			if (character->last_walk + delay >= now)
				continue;

			character->last_walk = now;

#define CHECK_WARP(test, x, y) (test && (warp = map->GetWarp(x, y)) && warp->levelreq <= character->level \
	&& (warp->spec == Map_Warp::Door|| warp->spec == Map_Warp::NoDoor))

			if (CHECK_WARP(true, character->x, character->y))
			{
				actions.push_back({character, warp->map, warp->x, warp->y});
			}
			else if (CHECK_WARP(character->x > 0, character->x - 1, character->y))
			{
				actions.push_back({character, warp->map, warp->x, warp->y});
			}
			else if (CHECK_WARP(character->x < map->width, character->x + 1, character->y))
			{
				actions.push_back({character, warp->map, warp->x, warp->y});
			}
			else if (CHECK_WARP(character->y > 0, character->x, character->y - 1))
			{
				actions.push_back({character, warp->map, warp->x, warp->y});
			}
			else if (CHECK_WARP(character->y, character->x, character->y + 1))
			{
				actions.push_back({character, warp->map, warp->x, warp->y});
			}
#undef CHECK_WARP
		}
	}

	UTIL_FOREACH(actions, act)
	{
		if (act.character->admin < ADMIN_GUIDE && world->GetMap(act.map)->evacuate_lock)
		{
			act.character->StatusMsg(world->i18n.Format("map_evacuate_block"));
			act.character->Refresh();
		}
		else
		{
			act.character->Warp(act.map, act.x, act.y);
		}
	}
}

void world_despawn_items(void *world_void)
{
	World *world = static_cast<World *>(world_void);

	UTIL_FOREACH(world->maps, map)
	{
		restart_loop:
		UTIL_FOREACH(map->items, item)
		{
			if (item->unprotecttime < (Timer::GetTime() - static_cast<double>(world->config["ItemDespawnRate"])))
			{
				map->DelItem(item->uid, 0);
				goto restart_loop;
			}
		}
	}
}

void world_timed_save(void *world_void)
{
	World *world = static_cast<World *>(world_void);

	UTIL_FOREACH(world->characters, character)
	{
		character->Save();
	}

	world->guildmanager->SaveAll();
}

World::World(std::array<std::string, 6> dbinfo, const Config &eoserv_config, const Config &admin_config)
	: i18n(eoserv_config.find("ServerLanguage")->second)
	, admin_count(0)
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
	this->db.Connect(engine, dbinfo[1], util::to_int(dbinfo[5]), dbinfo[2], dbinfo[3], dbinfo[4]);

	try
	{
		this->drops_config.Read(this->config["DropsFile"]);
		this->shops_config.Read(this->config["ShopsFile"]);
		this->arenas_config.Read(this->config["ArenasFile"]);
		this->formulas_config.Read(this->config["FormulasFile"]);
		this->home_config.Read(this->config["HomeFile"]);
		this->skills_config.Read(this->config["SkillsFile"]);
	}
	catch (std::runtime_error &e)
	{
		Console::Wrn(e.what());
	}

	this->eif = new EIF(this->config["EIF"]);
	this->enf = new ENF(this->config["ENF"]);
	this->esf = new ESF(this->config["ESF"]);
	this->ecf = new ECF(this->config["ECF"]);

	this->maps.resize(static_cast<int>(this->config["Maps"]));
	int loaded = 0;
	int npcs = 0;
	for (int i = 0; i < static_cast<int>(this->config["Maps"]); ++i)
	{
		this->maps[i] = new Map(i + 1, this);
		if (this->maps[i]->exists)
		{
			npcs += this->maps[i]->npcs.size();
			++loaded;
		}
	}
	Console::Out("%i/%i maps loaded.", loaded, this->maps.size());
	Console::Out("%i NPCs loaded.", npcs);

	short max_quest = 0;

	UTIL_CFOREACH(this->enf->data, npc)
	{
		if (npc->type == ENF::Quest)
			max_quest = std::max(max_quest, npc->vendor_id);
	}

	for (int i = 1; i < static_cast<int>(max_quest + 1); ++i)
	{
		try
		{
			std::shared_ptr<Quest> q = std::make_shared<Quest>(i, this);
			this->quests.insert(std::make_pair(i, std::move(q)));
		}
		catch (...)
		{

		}
	}
	Console::Out("%i quests loaded.", this->quests.size());

	this->last_character_id = 0;

	TimeEvent *event = new TimeEvent(world_spawn_npcs, this, 1.0, Timer::FOREVER);
	this->timer.Register(event);

	event = new TimeEvent(world_act_npcs, this, 0.05, Timer::FOREVER);
	this->timer.Register(event);

	if (int(this->config["RecoverSpeed"]) > 0)
	{
		event = new TimeEvent(world_recover, this, double(this->config["RecoverSpeed"]), Timer::FOREVER);
		this->timer.Register(event);
	}

	if (int(this->config["NPCRecoverSpeed"]) > 0)
	{
		event = new TimeEvent(world_npc_recover, this, double(this->config["NPCRecoverSpeed"]), Timer::FOREVER);
		this->timer.Register(event);
	}

	if (int(this->config["WarpSuck"]) > 0)
	{
		event = new TimeEvent(world_warp_suck, this, 1.0, Timer::FOREVER);
		this->timer.Register(event);
	}

	if (this->config["ItemDespawn"])
	{
		event = new TimeEvent(world_despawn_items, this, static_cast<double>(this->config["ItemDespawnCheck"]), Timer::FOREVER);
		this->timer.Register(event);
	}

	if (this->config["TimedSave"])
	{
		event = new TimeEvent(world_timed_save, this, static_cast<double>(this->config["TimedSave"]), Timer::FOREVER);
		this->timer.Register(event);
	}

	exp_table[0] = 0;
	for (std::size_t i = 1; i < this->exp_table.size(); ++i)
	{
		exp_table[i] = int(util::round(std::pow(double(i), 3.0) * 133.1));
	}

	for (std::size_t i = 0; i < this->boards.size(); ++i)
	{
		this->boards[i] = new Board(i);
	}

	this->guildmanager = new GuildManager(this);

	this->LoadHome();
}

void World::UpdateAdminCount(int admin_count)
{
	this->admin_count = admin_count;

	if (admin_count == 0 && this->config["FirstCharacterAdmin"])
	{
		Console::Out("There are no admin characters!");
		Console::Out("The next character created will be given HGM status!");
	}
}

void World::Command(std::string command, const std::vector<std::string>& arguments, Command_Source* from)
{
	std::unique_ptr<System_Command_Source> system_source;

	if (!from)
	{
		system_source.reset(new System_Command_Source(this));
		from = system_source.get();
	}

	Commands::Handle(util::lowercase(command), arguments, from);
}

void World::LoadHome()
{
	this->homes.clear();

	std::unordered_map<std::string, Home *> temp_homes;

	UTIL_FOREACH(this->home_config, hc)
	{
		std::vector<std::string> parts = util::explode('.', hc.first);

		if (parts.size() < 2)
		{
			continue;
		}

		if (parts[0] == "level")
		{
			int level = util::to_int(parts[1]);

			std::unordered_map<std::string, Home *>::iterator home_iter = temp_homes.find(hc.second);

			if (home_iter == temp_homes.end())
			{
				Home *home = new Home;
				home->id = static_cast<std::string>(hc.second);
				temp_homes[hc.second] = home;
				home->level = level;
			}
			else
			{
				home_iter->second->level = level;
			}

			continue;
		}

		Home *&home = temp_homes[parts[0]];

		if (!home)
		{
			temp_homes[parts[0]] = home = new Home;
			home->id = parts[0];
		}

		if (parts[1] == "name")
		{
			home->name = home->name = static_cast<std::string>(hc.second);
		}
		else if (parts[1] == "location")
		{
			std::vector<std::string> locparts = util::explode(',', hc.second);
			home->map = locparts.size() >= 1 ? util::to_int(locparts[0]) : 1;
			home->x = locparts.size() >= 2 ? util::to_int(locparts[1]) : 0;
			home->y = locparts.size() >= 3 ? util::to_int(locparts[2]) : 0;
		}
	}

	UTIL_FOREACH(temp_homes, home)
	{
		this->homes.push_back(home.second);
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
	UTIL_FOREACH(this->server->clients, client)
	{
		EOClient *eoclient = static_cast<EOClient *>(client);

		if (eoclient->id == lowest_free_id)
		{
			lowest_free_id = eoclient->id + 1;
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
	}

	this->characters.erase(
		std::remove(UTIL_RANGE(this->characters), character),
		this->characters.end()
	);
}

void World::Msg(Command_Source *from, std::string message, bool echo)
{
	std::string from_str = from ? from->SourceName() : "server";

	message = util::text_cap(message, static_cast<int>(this->config["ChatMaxWidth"]) - util::text_width(util::ucfirst(from_str) + "  "));

	PacketBuilder builder(PACKET_TALK, PACKET_MSG, 2 + from_str.length() + message.length());
	builder.AddBreakString(from_str);
	builder.AddBreakString(message);

	UTIL_FOREACH(this->characters, character)
	{
		if (!echo && character == from)
		{
			continue;
		}

		character->Send(builder);
	}
}

void World::AdminMsg(Command_Source *from, std::string message, int minlevel, bool echo)
{
	std::string from_str = from ? from->SourceName() : "server";

	message = util::text_cap(message, static_cast<int>(this->config["ChatMaxWidth"]) - util::text_width(util::ucfirst(from_str) + "  "));

	PacketBuilder builder(PACKET_TALK, PACKET_ADMIN, 2 + from_str.length() + message.length());
	builder.AddBreakString(from_str);
	builder.AddBreakString(message);

	UTIL_FOREACH(this->characters, character)
	{
		if ((!echo && character == from) || character->admin < minlevel)
		{
			continue;
		}

		character->Send(builder);
	}
}

void World::AnnounceMsg(Command_Source *from, std::string message, bool echo)
{
	std::string from_str = from ? from->SourceName() : "server";

	message = util::text_cap(message, static_cast<int>(this->config["ChatMaxWidth"]) - util::text_width(util::ucfirst(from_str) + "  "));

	PacketBuilder builder(PACKET_TALK, PACKET_ANNOUNCE, 2 + from_str.length() + message.length());
	builder.AddBreakString(from_str);
	builder.AddBreakString(message);

	UTIL_FOREACH(this->characters, character)
	{
		if (!echo && character == from)
		{
			continue;
		}

		character->Send(builder);
	}
}

void World::ServerMsg(std::string message)
{
	message = util::text_cap(message, static_cast<int>(this->config["ChatMaxWidth"]) - util::text_width("Server  "));

	PacketBuilder builder(PACKET_TALK, PACKET_SERVER, message.length());
	builder.AddString(message);

	UTIL_FOREACH(this->characters, character)
	{
		character->Send(builder);
	}
}

void World::AdminReport(Character *from, std::string reportee, std::string message)
{
	message = util::text_cap(message, static_cast<int>(this->config["ChatMaxWidth"]) - util::text_width(util::ucfirst(from->name) + "  reports: " + reportee + ", "));

	PacketBuilder builder(PACKET_ADMININTERACT, PACKET_REPLY, 5 + from->name.length() + message.length() + reportee.length());
	builder.AddChar(2); // message type
	builder.AddByte(255);
	builder.AddBreakString(from->name);
	builder.AddBreakString(message);
	builder.AddBreakString(reportee);

	UTIL_FOREACH(this->characters, character)
	{
		if (character->admin >= static_cast<int>(this->admin_config["reports"]))
		{
			character->Send(builder);
		}
	}

	short boardid = static_cast<int>(this->server->world->config["AdminBoard"]) - 1;

	if (static_cast<std::size_t>(boardid) < this->server->world->boards.size())
	{
		Board *admin_board = this->server->world->boards[boardid];

		Board_Post *newpost = new Board_Post;
		newpost->id = ++admin_board->last_id;
		newpost->author = from->name;
		newpost->author_admin = from->admin;
		newpost->subject = std::string(" [Report] ") + util::ucfirst(from->name) + " reports: " + reportee;
		newpost->body = message;
		newpost->time = Timer::GetTime();

		admin_board->posts.push_front(newpost);

		if (admin_board->posts.size() > static_cast<std::size_t>(static_cast<int>(this->server->world->config["AdminBoardLimit"])))
		{
			admin_board->posts.pop_back();
		}
	}
}

void World::AdminRequest(Character *from, std::string message)
{
	message = util::text_cap(message, static_cast<int>(this->config["ChatMaxWidth"]) - util::text_width(util::ucfirst(from->name) + "  needs help: "));

	PacketBuilder builder(PACKET_ADMININTERACT, PACKET_REPLY, 4 + from->name.length() + message.length());
	builder.AddChar(1); // message type
	builder.AddByte(255);
	builder.AddBreakString(from->name);
	builder.AddBreakString(message);

	UTIL_FOREACH(this->characters, character)
	{
		if (character->admin >= static_cast<int>(this->admin_config["reports"]))
		{
			character->Send(builder);
		}
	}

	short boardid = static_cast<int>(this->server->world->config["AdminBoard"]) - 1;

	if (static_cast<std::size_t>(boardid) < this->server->world->boards.size())
	{
		Board *admin_board = this->server->world->boards[boardid];

		Board_Post *newpost = new Board_Post;
		newpost->id = ++admin_board->last_id;
		newpost->author = from->name;
		newpost->author_admin = from->admin;
		newpost->subject = std::string(" [Request] ") + util::ucfirst(from->name) + " needs help";
		newpost->body = message;
		newpost->time = Timer::GetTime();

		admin_board->posts.push_front(newpost);

		if (admin_board->posts.size() > static_cast<std::size_t>(static_cast<int>(this->server->world->config["AdminBoardLimit"])))
		{
			admin_board->posts.pop_back();
		}
	}
}

void World::Rehash()
{
	try
	{
		this->config.Read("config.ini");
		this->admin_config.Read("admin.ini");
		this->drops_config.Read(this->config["DropsFile"]);
		this->shops_config.Read(this->config["ShopsFile"]);
		this->arenas_config.Read(this->config["ArenasFile"]);
		this->formulas_config.Read(this->config["FormulasFile"]);
		this->home_config.Read(this->config["HomeFile"]);
		this->skills_config.Read(this->config["SkillsFile"]);
	}
	catch (std::runtime_error &e)
	{
		Console::Err(e.what());
	}

	this->LoadHome();

	UTIL_FOREACH(this->maps, map)
	{
		map->LoadArena();

		UTIL_FOREACH(map->npcs, npc)
		{
			npc->LoadShopDrop();
		}
	}
}

void World::ReloadPub()
{
	this->eif->Read(this->config["EIF"]);
	this->enf->Read(this->config["ENF"]);
	this->esf->Read(this->config["ESF"]);
	this->ecf->Read(this->config["ECF"]);

	std::string filename;
	std::FILE *fh;
	InitReply replycode;

	for (int i = 0; i < 4; ++i)
	{
		std::string content;

		switch (i)
		{
			case 0: filename = static_cast<std::string>(this->config["EIF"]); replycode = INIT_FILE_EIF; break;
			case 1: filename = static_cast<std::string>(this->config["ENF"]); replycode = INIT_FILE_ENF; break;
			case 2: filename = static_cast<std::string>(this->config["ESF"]); replycode = INIT_FILE_ESF; break;
			case 3: filename = static_cast<std::string>(this->config["ECF"]); replycode = INIT_FILE_ECF; break;
		}

		fh = std::fopen(filename.c_str(), "rb");

		if (!fh)
		{
			Console::Err("Could not load file: %s", filename.c_str());
			std::exit(1);
		}

		do {
			char buf[4096];
			int len = std::fread(buf, sizeof(char), 4096, fh);
			content.append(buf, len);
		} while (!std::feof(fh));

		std::fclose(fh);

		PacketBuilder builder(PACKET_F_INIT, PACKET_A_INIT, 2 + content.length());
		builder.AddChar(replycode);
		builder.AddChar(1); // fileid
		builder.AddString(content);

		UTIL_FOREACH(this->characters, character)
		{
			character->Send(builder);
		}
	}

	UTIL_FOREACH(this->characters, character)
	{
		character->Warp(character->mapid, character->x, character->y);
	}
}

Character *World::GetCharacter(std::string name)
{
	name = util::lowercase(name);

	UTIL_FOREACH(this->characters, character)
	{
		if (character->name == name)
		{
			return character;
		}
	}

	return 0;
}

Character *World::GetCharacterPID(unsigned int id)
{
	UTIL_FOREACH(this->characters, character)
	{
		if (character->player->id == id)
		{
			return character;
		}
	}

	return 0;
}

Character *World::GetCharacterCID(unsigned int id)
{
	UTIL_FOREACH(this->characters, character)
	{
		if (character->id == id)
		{
			return character;
		}
	}

	return 0;
}

Map *World::GetMap(short id)
{
	try
	{
		return this->maps.at(id - 1);
	}
	catch (...)
	{
		try
		{
			return this->maps.at(0);
		}
		catch (...)
		{
			throw std::runtime_error("Map #" + util::to_string(id) + " and fallback map #1 are unavailable");
		}
	}
}

Home *World::GetHome(Character *character)
{
	Home *home = 0;
	static Home *null_home = new Home;

	UTIL_FOREACH(this->homes, h)
	{
		if (h->id == character->home)
		{
			return h;
		}
	}

	int current_home_level = -2;
	UTIL_FOREACH(this->homes, h)
	{
		if (h->level <= character->level && h->level > current_home_level)
		{
			home = h;
			current_home_level = h->level;
		}
	}

	if (!home)
	{
		home = null_home;
	}

	return home;
}

Home *World::GetHome(std::string id)
{
	UTIL_FOREACH(this->homes, h)
	{
		if (h->id == id)
		{
			return h;
		}
	}

	return 0;
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

	if (static_cast<int>(this->config["StartMap"]))
	{
		startmapinfo = ", `map`, `x`, `y`";
		std::snprintf(buffer, 1024, ",%i,%i,%i", static_cast<int>(this->config["StartMap"]), static_cast<int>(this->config["StartX"]), static_cast<int>(this->config["StartY"]));
		startmapval = buffer;
	}

	this->db.Query("INSERT INTO `characters` (`name`, `account`, `gender`, `hairstyle`, `haircolor`, `race`, `inventory`, `bank`, `paperdoll`, `spells`, `quest`, `vars`@) VALUES ('$','$',#,#,#,#,'$','','$','$','',''@)",
		startmapinfo.c_str(), name.c_str(), player->username.c_str(), gender, hairstyle, haircolor, race,
		static_cast<std::string>(this->config["StartItems"]).c_str(), static_cast<std::string>(gender?this->config["StartEquipMale"]:this->config["StartEquipFemale"]).c_str(),
		static_cast<std::string>(this->config["StartSpells"]).c_str(), startmapval.c_str());

	return new Character(name, this);
}

void World::DeleteCharacter(std::string name)
{
	this->db.Query("DELETE FROM `characters` WHERE name = '$'", name.c_str());
}

Player *World::Login(const std::string& username, util::secure_string&& password)
{
	if (LoginCheck(username, std::move(password)) == LOGIN_WRONG_USERPASS)
		return 0;

	return new Player(username, this);
}

Player *World::Login(std::string username)
{
	return new Player(username, this);
}

LoginReply World::LoginCheck(const std::string& username, util::secure_string&& password)
{
	{
		util::secure_string password_buffer(std::move(std::string(this->config["PasswordSalt"]) + username + password.str()));
		password = sha256(password_buffer.str());
	}

	Database_Result res = this->db.Query("SELECT 1 FROM `accounts` WHERE `username` = '$' AND `password` = '$'", username.c_str(), password.str().c_str());

	if (res.empty())
	{
		return LOGIN_WRONG_USERPASS;
	}
	else if (this->PlayerOnline(username))
	{
		return LOGIN_LOGGEDIN;
	}
	else
	{
		return LOGIN_OK;
	}
}

bool World::CreatePlayer(const std::string& username, util::secure_string&& password,
	const std::string& fullname, const std::string& location, const std::string& email,
	const std::string& computer, const std::string& hdid, const std::string& ip)
{
	{
		util::secure_string password_buffer(std::move(std::string(this->config["PasswordSalt"]) + username + password.str()));
		password = sha256(password_buffer.str());
	}

	Database_Result result = this->db.Query("INSERT INTO `accounts` (`username`, `password`, `fullname`, `location`, `email`, `computer`, `hdid`, `regip`, `created`) VALUES ('$','$','$','$','$','$','$','$',#)",
		username.c_str(), password.str().c_str(), fullname.c_str(), location.c_str(), email.c_str(), computer.c_str(), hdid.c_str(), ip.c_str(), std::time(0));

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

	UTIL_FOREACH(this->server->clients, client)
	{
		EOClient *eoclient = static_cast<EOClient *>(client);

		if (eoclient->player)
		{
			if (eoclient->player->username.compare(username) == 0)
			{
				return true;
			}
		}
	}

	return false;
}

void World::Kick(Command_Source *from, Character *victim, bool announce)
{
	if (announce)
		this->ServerMsg(i18n.Format("announce_removed", victim->name, from ? from->SourceName() : "server", i18n.Format("kicked")));

	victim->player->client->Close();
}

void World::Jail(Command_Source *from, Character *victim, bool announce)
{
	if (announce)
		this->ServerMsg(i18n.Format("announce_removed", victim->name, from ? from->SourceName() : "server", i18n.Format("jailed")));

	victim->Warp(static_cast<int>(this->config["JailMap"]), static_cast<int>(this->config["JailX"]), static_cast<int>(this->config["JailY"]), this->config["WarpBubbles"] ? WARP_ANIMATION_ADMIN : WARP_ANIMATION_NONE);
}

void World::Ban(Command_Source *from, Character *victim, int duration, bool announce)
{
	if (announce)
		this->ServerMsg(i18n.Format("announce_removed", victim->name, from ? from->SourceName() : "server", i18n.Format("banned")));

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
		query += ", '" + db.Escape(from->SourceName()) + "')";
	}
	else
	{
		query += ")";
	}

	db.Query(query.c_str(), std::time(0));

	victim->player->client->Close();
}

void World::Mute(Command_Source *from, Character *victim, bool announce)
{
	if (announce && !this->config["SilentMute"])
		this->ServerMsg(i18n.Format("announce_muted", victim->name, from ? from->SourceName() : "server", i18n.Format("banned")));

	victim->Mute(from);
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

World::~World()
{
	std::list<Character *> todelete;

	UTIL_FOREACH(this->characters, character)
	{
		todelete.push_back(character);
	}

	UTIL_FOREACH(todelete, character)
	{
		character->player->client->Close(true);
		delete character;
	}

	delete this->guildmanager;
}
