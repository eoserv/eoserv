
Map::Map(int id)
{
	char namebuf[6];
	std::string filename = "./data/maps/";
	std::sprintf(namebuf, "%05i", std::abs(id));
	this->filename = filename;
	filename.append(namebuf);
	filename.append(".emf");

	std::FILE *fh = std::fopen(filename.c_str(), "rb");

	if (!fh)
	{
		std::printf("Could not load file: %s\n", filename.c_str());
		return;
	}

	std::fseek(fh, 3, SEEK_SET);
	std::fread(this->rid, sizeof(char), 4, fh);

	std::fclose(fh);
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
	PacketBuilder builder;

	builder.SetID(PACKET_PLAYERS, PACKET_REMOVE);
	builder.AddShort(character->player->id);

	UTIL_FOREACH(characters, checkcharacter)
	{
		if (checkcharacter == character)
		{
			continue;
		}

		checkcharacter->player->client->SendBuilder(builder);
	}

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

	builder.SetID(PACKET_PLAYERS, PACKET_AGREE);

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

void Map::Attack(Character *from, int direction)
{
	PacketBuilder builder;

	from->direction = direction;

	builder.SetID(PACKET_ATTACK, PACKET_PLAYER);
	builder.AddShort(from->player->id);
	builder.AddChar(direction);

	UTIL_FOREACH(characters, character)
	{
		if (character == from)
		{
			continue;
		}

		character->player->client->SendBuilder(builder);
	}
}

void Map::Face(Character *from, int direction)
{
	PacketBuilder builder;

	from->direction = direction;

	builder.SetID(PACKET_FACE, PACKET_PLAYER);
	builder.AddShort(from->player->id);
	builder.AddChar(direction);

	UTIL_FOREACH(characters, character)
	{
		if (character == from)
		{
			continue;
		}

		character->player->client->SendBuilder(builder);
	}
}

void Map::Sit(Character *from)
{
	PacketBuilder builder;

	from->sitting = true;

	builder.SetID(PACKET_SIT, PACKET_PLAYER);
	builder.AddShort(from->player->id);
	builder.AddChar(13); // ?
	builder.AddChar(29); // ?
	builder.AddChar(2); // direction?
	builder.AddChar(0); // ?

	UTIL_FOREACH(characters, character)
	{
		character->player->client->SendBuilder(builder);
	}
}

void Map::Stand(Character *from)
{
	PacketBuilder builder;

	from->sitting = false;

	builder.SetID(PACKET_SIT, PACKET_REMOVE);
	builder.AddShort(from->player->id);
	builder.AddChar(13); // ?
	builder.AddChar(29); // ?

	UTIL_FOREACH(characters, character)
	{
		character->player->client->SendBuilder(builder);
	}
}

void Map::Emote(Character *from, int direction)
{
	PacketBuilder builder;

	from->direction = direction;

	builder.SetID(PACKET_EMOTE, PACKET_PLAYER);
	builder.AddShort(from->player->id);
	builder.AddChar(direction);

	UTIL_FOREACH(characters, character)
	{
		if (character == from)
		{
			continue;
		}

		character->player->client->SendBuilder(builder);
	}
}
