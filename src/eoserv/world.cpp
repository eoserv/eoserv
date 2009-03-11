
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
	std::printf("%i maps loaded.\n", this->maps.size());

	the_world = this;

	eoserv_items = new EIF("./data/pub/dat001.eif");
	eoserv_npcs = new ENF("./data/pub/dtn001.enf");
	eoserv_spells = new ESF("./data/pub/dsl001.esf");
	eoserv_classes = new ECF("./data/pub/dat001.ecf");

	eoserv_config = config;
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
