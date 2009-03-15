
Map::Map(int id)
{
	char namebuf[6];
	if (id < 0)
	{
		return;
	}
	std::string filename = "./data/maps/";
	std::sprintf(namebuf, "%05i", id);
	this->filename = filename;
	filename.append(namebuf);
	filename.append(".emf");

	std::FILE *fh = std::fopen(filename.c_str(), "rb");

	if (!fh)
	{
		std::fprintf(stderr, "Could not load file: %s\n", filename.c_str());
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
	builder.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Boots]));
	builder.AddShort(0); // ??
	builder.AddShort(0); // ??
	builder.AddShort(0); // ??
	builder.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Armor]));
	builder.AddShort(0); // ??
	builder.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Hat]));
	builder.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Shield]));
	builder.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Weapon]));
	builder.AddChar(character->sitting);
	builder.AddChar(0); // visible
	builder.AddByte(255);
	builder.AddChar(1); // 0 = NPC, 1 = player

	UTIL_FOREACH(this->characters, checkcharacter)
	{
		if (checkcharacter == character || !character->InRange(checkcharacter))
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

	UTIL_FOREACH(this->characters, checkcharacter)
	{
		if (checkcharacter == character || !character->InRange(checkcharacter))
		{
			continue;
		}

		checkcharacter->player->client->SendBuilder(builder);
	}

	UTIL_FOREACH(this->characters, checkcharacter)
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
		if (character == from || !from->InRange(character))
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

	int newx;
	int newy;
	int oldx;
	int oldy;
	std::list<Character *> newchars;
	std::list<Character *> oldchars;
/*
1237068321 [RACTION ] Unknown (17_18) - Report iipandal reports wwwwwwwwewww for
 Spamming and disconnecting.
1237068321 [RECV >>E] 71 17 18 82 101 112 111 114 116 255 105 105 112 97 110 100
 97 108 32 114 101 112 111 114 116 115 32 119 119 119 119 119 119 119 119 101 11
9 119 119 32 102 111 114 32 83 112 97 109 109 105 110 103 32 97 110 100 32 100 1
05 115 99 111 110 110 101 99 116 105 110 103 46 126
*/
	UTIL_FOREACH(this->characters, checkchar)
	{
		if (checkchar == from)
		{
			continue;
		}
		switch (direction)
		{
			case DIRECTION_UP:
				for (int i = -11; i <= 11; ++i)
				{
					newy = from->y - 11 + std::abs(i);
					newx = from->x + i;
					oldy = from->y + 12 - std::abs(i);
					oldx = from->x + i;

					if (checkchar->x == oldx && checkchar->y == oldy)
					{
						oldchars.push_back(checkchar);
					}
					else if (checkchar->x == newx && checkchar->y == newy)
					{
						newchars.push_back(checkchar);
					}
				}
				break;

			case DIRECTION_RIGHT:
				for (int i = -11; i <= 11; ++i)
				{
					newx = from->x + 11 - std::abs(i);
					newy = from->y + i;
					oldx = from->x - 12 + std::abs(i);
					oldy = from->y + i;

					if (checkchar->x == oldx && checkchar->y == oldy)
					{
						oldchars.push_back(checkchar);
					}
					else if (checkchar->x == newx && checkchar->y == newy)
					{
						newchars.push_back(checkchar);
					}
				}
				break;

			case DIRECTION_DOWN:
				for (int i = -11; i <= 11; ++i)
				{
					newy = from->y + 11 - std::abs(i);
					newx = from->x + i;
					oldy = from->y - 12 + std::abs(i);
					oldx = from->x + i;

					if (checkchar->x == oldx && checkchar->y == oldy)
					{
						oldchars.push_back(checkchar);
					}
					else if (checkchar->x == newx && checkchar->y == newy)
					{
						newchars.push_back(checkchar);
					}
				}
				break;

			case DIRECTION_LEFT:
				for (int i = -11; i <= 11; ++i)
				{
					newx = from->x - 11 + std::abs(i);
					newy = from->y + i;
					oldx = from->x + 12 - std::abs(i);
					oldy = from->y + i;

					if (checkchar->x == oldx && checkchar->y == oldy)
					{
						oldchars.push_back(checkchar);
					}
					else if (checkchar->x == newx && checkchar->y == newy)
					{
						newchars.push_back(checkchar);
					}
				}
				break;

		}
	}

	from->direction = direction;

	builder.SetID(PACKET_PLAYERS, PACKET_REMOVE);
	builder.AddShort(from->player->id);

	UTIL_FOREACH(oldchars, character)
	{
		PacketBuilder rbuilder;
		rbuilder.SetID(PACKET_PLAYERS, PACKET_REMOVE);
		rbuilder.AddShort(character->player->id);

		character->player->client->SendBuilder(builder);
		from->player->client->SendBuilder(rbuilder);
	}

	builder.Reset();

	builder.SetID(PACKET_PLAYERS, PACKET_AGREE);
	builder.AddByte(255);
	builder.AddBreakString(from->name);
	builder.AddShort(from->player->id);
	builder.AddShort(from->mapid);
	builder.AddShort(from->x);
	builder.AddShort(from->y);
	builder.AddChar(from->direction);
	builder.AddChar(6); // ?
	builder.AddString("SEX"); // guild tag
	builder.AddChar(from->level);
	builder.AddChar(from->gender);
	builder.AddChar(from->hairstyle);
	builder.AddChar(from->haircolor);
	builder.AddChar(from->race);
	builder.AddShort(from->maxhp);
	builder.AddShort(from->hp);
	builder.AddShort(from->maxtp);
	builder.AddShort(from->tp);
	// equipment
	builder.AddShort(eoserv_items->GetDollGraphic(from->paperdoll[Character::Boots]));
	builder.AddShort(0); // ??
	builder.AddShort(0); // ??
	builder.AddShort(0); // ??
	builder.AddShort(eoserv_items->GetDollGraphic(from->paperdoll[Character::Armor]));
	builder.AddShort(0); // ??
	builder.AddShort(eoserv_items->GetDollGraphic(from->paperdoll[Character::Hat]));
	builder.AddShort(eoserv_items->GetDollGraphic(from->paperdoll[Character::Shield]));
	builder.AddShort(eoserv_items->GetDollGraphic(from->paperdoll[Character::Weapon]));
	builder.AddChar(from->sitting);
	builder.AddChar(0); // visible
	builder.AddByte(255);
	builder.AddChar(1); // 0 = NPC, 1 = player

	UTIL_FOREACH(newchars, character)
	{
		PacketBuilder rbuilder;
		rbuilder.SetID(PACKET_PLAYERS, PACKET_AGREE);
		rbuilder.AddByte(255);
		rbuilder.AddBreakString(character->name);
		rbuilder.AddShort(character->player->id);
		rbuilder.AddShort(character->mapid);
		rbuilder.AddShort(character->x);
		rbuilder.AddShort(character->y);
		rbuilder.AddChar(character->direction);
		rbuilder.AddChar(6); // ?
		rbuilder.AddString("SEX"); // guild tag
		rbuilder.AddChar(character->level);
		rbuilder.AddChar(character->gender);
		rbuilder.AddChar(character->hairstyle);
		rbuilder.AddChar(character->haircolor);
		rbuilder.AddChar(character->race);
		rbuilder.AddShort(character->maxhp);
		rbuilder.AddShort(character->hp);
		rbuilder.AddShort(character->maxtp);
		rbuilder.AddShort(character->tp);
		// equipment
		rbuilder.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Boots]));
		rbuilder.AddShort(0); // ??
		rbuilder.AddShort(0); // ??
		rbuilder.AddShort(0); // ??
		rbuilder.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Armor]));
		rbuilder.AddShort(0); // ??
		rbuilder.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Hat]));
		rbuilder.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Shield]));
		rbuilder.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Weapon]));
		rbuilder.AddChar(character->sitting);
		rbuilder.AddChar(0); // visible
		rbuilder.AddByte(255);
		rbuilder.AddChar(1); // 0 = NPC, 1 = player

		character->player->client->SendBuilder(builder);
		from->player->client->SendBuilder(rbuilder);
	}

	builder.Reset();

	builder.SetID(PACKET_WALK, PACKET_PLAYER);
	builder.AddShort(from->player->id);
	builder.AddChar(direction);
	builder.AddChar(from->x);
	builder.AddChar(from->y);

	UTIL_FOREACH(characters, character)
	{
		if (character == from || !from->InRange(character))
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
		if (character == from || !from->InRange(character))
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
		if (character == from || !from->InRange(character))
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
		if (!from->InRange(character))
		{
			continue;
		}

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
		if (!from->InRange(character))
		{
			continue;
		}

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
		if (character == from || !from->InRange(character))
		{
			continue;
		}

		character->player->client->SendBuilder(builder);
	}
}
