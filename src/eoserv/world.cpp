
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

	this->maps.resize(279);
	this->maps[0] = new Map(1); // Just in case
	for (int i = 1; i <= 278; ++i)
	{
		this->maps[i] = new Map(i);
	}
	std::printf("%i maps loaded.\n", this->maps.size()-1);

	the_world = this;

	eoserv_items = new EIF("./data/pub/dat001.eif");
	eoserv_npcs = new ENF("./data/pub/dtn001.enf");
	eoserv_spells = new ESF("./data/pub/dsl001.esf");
	eoserv_classes = new ECF("./data/pub/dat001.ecf");

	Config aconfig;

	try
	{
		Config load_config("admin.ini");
		aconfig = load_config;
	}
	catch (std::runtime_error)
	{
		std::fputs("WARNING: Could not load admin.ini - using defaults\n", stderr);
	}

#define CONFIG_DEFAULT(key, value)\
if (config.find(key) == config.end())\
{\
	util::variant vv = util::variant(value);\
	config[key] = vv;\
	std::fprintf(stderr, "WARNING: Could not load admin config value '%s' - using default (%s)\n", key, static_cast<std::string>(vv).c_str());\
}
	CONFIG_DEFAULT("item"          , 1);
	CONFIG_DEFAULT("npc"           , 1);
	CONFIG_DEFAULT("spell"         , 1);
	CONFIG_DEFAULT("class"         , 1);
	CONFIG_DEFAULT("info"          , 1);
	CONFIG_DEFAULT("kick"          , 1);
	CONFIG_DEFAULT("skick"         , 3);
	CONFIG_DEFAULT("jail"          , 1);
	CONFIG_DEFAULT("sjail"         , 3);
	CONFIG_DEFAULT("ban"           , 2);
	CONFIG_DEFAULT("sban"          , 3);
	CONFIG_DEFAULT("warp"          , 2);
	CONFIG_DEFAULT("warptome"      , 2);
	CONFIG_DEFAULT("warpmeto"      , 2);
	CONFIG_DEFAULT("evacuate"      , 2);
	CONFIG_DEFAULT("sitem"         , 4);
	CONFIG_DEFAULT("ditem"         , 4);
	CONFIG_DEFAULT("learn"         , 3);
	CONFIG_DEFAULT("setlevel"      , 3);
	CONFIG_DEFAULT("setexp"        , 3);
	CONFIG_DEFAULT("setstr"        , 3);
	CONFIG_DEFAULT("setint"        , 3);
	CONFIG_DEFAULT("setwis"        , 3);
	CONFIG_DEFAULT("setagi"        , 3);
	CONFIG_DEFAULT("setcon"        , 3);
	CONFIG_DEFAULT("setcha"        , 3);
	CONFIG_DEFAULT("setstatpoints" , 3);
	CONFIG_DEFAULT("setskillpoints", 3);
	CONFIG_DEFAULT("settitle"      , 3);
	CONFIG_DEFAULT("setpartner"    , 3);
	CONFIG_DEFAULT("sethome"       , 3);
	CONFIG_DEFAULT("sethomemap"    , 3);
	CONFIG_DEFAULT("sethomex"      , 3);
	CONFIG_DEFAULT("sethomey"      , 3);
#undef CONFIG_DEFAULT

	eoserv_config = config;
	admin_config = aconfig;

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
	UTIL_FOREACH(this->server->clients, client)
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

void World::AdminMsg(Character *from, std::string message)
{
	PacketBuilder builder;

	builder.SetID(PACKET_TALK, PACKET_MOVEADMIN);
	builder.AddBreakString(from->name);
	builder.AddBreakString(message);

	UTIL_FOREACH(this->characters, character)
	{
		if (character == from || character->admin < ADMIN_GUARDIAN)
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

Character *World::GetCharacter(std::string name)
{
	Character *selected = 0;

	std::transform(name.begin(), name.end(), name.begin(), static_cast<int(*)(int)>(std::tolower));

	UTIL_FOREACH(this->characters, character)
	{
		if (character->name.compare(name) == 0)
		{
			selected = character;
			break;
		}
	}

	return selected;
}

void World::Kick(Character *from, Character *victim)
{
	victim->player->client->Close();
}
