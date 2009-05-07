
World::World(util::array<std::string, 5> dbinfo, Config config)
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
	CONFIG_DEFAULT("sitem"         , 4)
	CONFIG_DEFAULT("ditem"         , 4)
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
#undef CONFIG_DEFAULT

	eoserv_config = config;
	admin_config = aconfig;

	eoserv_items = new EIF(static_cast<std::string>(eoserv_config["EIF"]));
	eoserv_npcs = new ENF(static_cast<std::string>(eoserv_config["ENF"]));
	eoserv_spells = new ESF(static_cast<std::string>(eoserv_config["ESF"]));
	eoserv_classes = new ECF(static_cast<std::string>(eoserv_config["ECF"]));

	this->maps.resize(static_cast<int>(eoserv_config["Maps"])+1);
	this->maps[0] = new Map(1); // Just in case
	int loaded = 0;
	for (int i = 1; i <= static_cast<int>(eoserv_config["Maps"]); ++i)
	{
		this->maps[i] = new Map(i);
		if (this->maps[i]->exists)
		{
			++loaded;
		}
	}
	std::printf("%i/%i maps loaded.\n", loaded, this->maps.size()-1);

	the_world = this;

	this->last_character_id = 0;
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
	UTIL_LIST_IFOREACH(this->characters.begin(), this->characters.end(), Character *, checkcharacter)
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

	UTIL_LIST_FOREACH_ALL(this->characters, Character *, character)
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

	builder.SetID(PACKET_TALK, PACKET_MOVEADMIN);
	if (from)
	{
		builder.AddBreakString(from->name);
	}
	else
	{
		builder.AddBreakString("Server");
	}
	builder.AddBreakString(message);

	UTIL_LIST_FOREACH_ALL(this->characters, Character *, character)
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

	UTIL_LIST_FOREACH_ALL(this->characters, Character *, character)
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

	UTIL_LIST_FOREACH_ALL(this->characters, Character *, character)
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

	UTIL_LIST_FOREACH_ALL(this->characters, Character *, character)
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

	UTIL_LIST_FOREACH_ALL(this->characters, Character *, character)
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
