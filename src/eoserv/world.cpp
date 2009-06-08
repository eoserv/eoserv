
void world_spawn_npcs(void *world_void)
{
	World *world = static_cast<World *>(world_void);

	double spawnrate = static_cast<double>(eoserv_config["SpawnRate"]) / 100.0;
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
		character->hp += character->maxhp / 10;
		character->tp += character->maxtp / 10;

		character->hp = std::min(character->hp, character->maxhp);
		character->tp = std::min(character->tp, character->maxtp);

		builder.Reset();
		builder.AddShort(character->hp);
		builder.AddShort(character->tp);
		builder.AddShort(0); // ?
		character->player->client->SendBuilder(builder);
	}
}

World::World(util::array<std::string, 5> dbinfo, Config config)
{
	if (int(this->timer.resolution * 1000.0) < 1)
	{
		std::printf("Timers set at approx. %i ms resolution\n", int(this->timer.resolution * 1000.0));
	}

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


	Config aconfig;

	try
	{
		aconfig.Read("admin.ini");
	}
	catch (std::runtime_error)
	{
		std::fputs("WARNING: Could not load admin.ini - using defaults\n", stderr);
	}

#define CONFIG_DEFAULT(key, value) if (aconfig.find(key) == aconfig.end()){aconfig[key] = util::variant(value); std::fprintf(stderr, "WARNING: Could not load admin config value '%s' - using default (%s)\n", key, static_cast<std::string>(aconfig[key]).c_str());}
	CONFIG_DEFAULT("item"          , 1)
	CONFIG_DEFAULT("npc"           , 1)
	CONFIG_DEFAULT("spell"         , 1)
	CONFIG_DEFAULT("class"         , 1)
	CONFIG_DEFAULT("info"          , 1)
	CONFIG_DEFAULT("kick"          , 1)
	CONFIG_DEFAULT("skick"         , 3)
	CONFIG_DEFAULT("jail"          , 1)
	CONFIG_DEFAULT("sjail"         , 3)
	CONFIG_DEFAULT("ban"           , 2)
	CONFIG_DEFAULT("sban"          , 3)
	CONFIG_DEFAULT("warp"          , 2)
	CONFIG_DEFAULT("warptome"      , 2)
	CONFIG_DEFAULT("warpmeto"      , 2)
	CONFIG_DEFAULT("evacuate"      , 2)
	CONFIG_DEFAULT("shutdown"      , 4)
	CONFIG_DEFAULT("rehash"        , 4)
	CONFIG_DEFAULT("sitem"         , 3)
	CONFIG_DEFAULT("ditem"         , 3)
	CONFIG_DEFAULT("learn"         , 3)
	CONFIG_DEFAULT("quake"         , 2)
	CONFIG_DEFAULT("setlevel"      , 3)
	CONFIG_DEFAULT("setexp"        , 3)
	CONFIG_DEFAULT("setstr"        , 3)
	CONFIG_DEFAULT("setint"        , 3)
	CONFIG_DEFAULT("setwis"        , 3)
	CONFIG_DEFAULT("setagi"        , 3)
	CONFIG_DEFAULT("setcon"        , 3)
	CONFIG_DEFAULT("setcha"        , 3)
	CONFIG_DEFAULT("setstatpoints" , 3)
	CONFIG_DEFAULT("setskillpoints", 3)
	CONFIG_DEFAULT("settitle"      , 3)
	CONFIG_DEFAULT("setpartner"    , 3)
	CONFIG_DEFAULT("sethome"       , 3)
	CONFIG_DEFAULT("sethomemap"    , 3)
	CONFIG_DEFAULT("sethomex"      , 3)
	CONFIG_DEFAULT("sethomey"      , 3)
	CONFIG_DEFAULT("setgender"     , 3)
	CONFIG_DEFAULT("sethairstyle"  , 3)
	CONFIG_DEFAULT("sethaircolor"  , 3)
	CONFIG_DEFAULT("setrace"       , 3)
	CONFIG_DEFAULT("setguild"      , 3)
	CONFIG_DEFAULT("setguildrank"  , 3)
	CONFIG_DEFAULT("setkarma"      , 3)
	CONFIG_DEFAULT("strip"         , 3)
	CONFIG_DEFAULT("killnpc"       , 2)
#undef CONFIG_DEFAULT

	eoserv_config = config;
	admin_config = aconfig;

	try
	{
		drops_config.Read(static_cast<std::string>(eoserv_config["DropsFile"]));
	}
	catch (std::runtime_error)
	{
		std::fprintf(stderr, "WARNING: Could not load %s\n", static_cast<std::string>(eoserv_config["DropsFile"]).c_str());
	}

	try
	{
		shops_config.Read(static_cast<std::string>(eoserv_config["ShopsFile"]));
	}
	catch (std::runtime_error)
	{
		std::fprintf(stderr, "WARNING: Could not load %s\n", static_cast<std::string>(eoserv_config["ShopsFile"]).c_str());
	}

	eoserv_items = new EIF(static_cast<std::string>(eoserv_config["EIF"]));
	eoserv_npcs = new ENF(static_cast<std::string>(eoserv_config["ENF"]));
	eoserv_spells = new ESF(static_cast<std::string>(eoserv_config["ESF"]));
	eoserv_classes = new ECF(static_cast<std::string>(eoserv_config["ECF"]));

	this->maps.resize(static_cast<int>(eoserv_config["Maps"])+1);
	this->maps[0] = new Map(1); // Just in case
	int loaded = 0;
	int npcs = 0;
	for (int i = 1; i <= static_cast<int>(eoserv_config["Maps"]); ++i)
	{
		this->maps[i] = new Map(i);
		if (this->maps[i]->exists)
		{
			npcs += this->maps[i]->npcs.size();
			++loaded;
		}
	}
	std::printf("%i/%i maps loaded.\n", loaded, this->maps.size()-1);
	std::printf("%i NPCs loaded.\n", npcs);
	the_world = this;

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

void World::Kick(Character *from, Character *victim, bool announce)
{
	victim->player->client->Close();
}

void World::Ban(Character *from, Character *victim, double duration, bool announce)
{
	this->server->AddBan(victim->player->username, victim->player->client->GetRemoteAddr(), victim->player->client->hdid, duration);
	victim->player->client->Close();
}
