
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

	this->exists = true;
	if (!fh)
	{
		this->exists = false;
		std::fprintf(stderr, "Could not load file: %s\n", filename.c_str());
		return;
	}

	std::fseek(fh, 0, SEEK_END);
	this->filesize = std::ftell(fh);

	std::fseek(fh, 3, SEEK_SET);
	std::fread(this->rid, sizeof(char), 4, fh);

	std::fclose(fh);
}

int Map::GenerateItemID()
{
	int lowest_free_id = 1;
	restart_loop:
	UTIL_FOREACH(this->items, item)
	{
		if (item.uid == lowest_free_id)
		{
			lowest_free_id = item.uid + 1;
			goto restart_loop;
		}
	}
	return lowest_free_id;
}

void Map::Enter(Character *character, int animation)
{
	PacketBuilder builder;
	this->characters.push_back(character);
	character->map = this;

	// TODO: How do I set the animation here?

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
	builder.AddShort(eoserv_items->Get(character->paperdoll[Character::Boots])->dollgraphic);
	builder.AddShort(0); // ??
	builder.AddShort(0); // ??
	builder.AddShort(0); // ??
	builder.AddShort(eoserv_items->Get(character->paperdoll[Character::Armor])->dollgraphic);
	builder.AddShort(0); // ??
	builder.AddShort(eoserv_items->Get(character->paperdoll[Character::Hat])->dollgraphic);
	builder.AddShort(eoserv_items->Get(character->paperdoll[Character::Shield])->dollgraphic);
	builder.AddShort(eoserv_items->Get(character->paperdoll[Character::Weapon])->dollgraphic);
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

void Map::Leave(Character *character, int animation)
{
	PacketBuilder builder;

	if (animation == WARP_ANIMATION_NONE)
	{
		builder.SetID(PACKET_PLAYERS, PACKET_REMOVE);
	}
	else
	{
		builder.SetID(PACKET_CLOTHES, PACKET_REMOVE);
	}
	builder.AddShort(character->player->id);
	if (animation != WARP_ANIMATION_NONE)
	{
		builder.AddChar(animation);
	}

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
	int seedistance = eoserv_config["SeeDistance"];

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
	std::list<Map_Item> newitems;
	UTIL_FOREACH(this->characters, checkchar)
	{
		if (checkchar == from)
		{
			continue;
		}
		switch (direction)
		{
			case DIRECTION_UP:
				for (int i = -seedistance; i <= seedistance; ++i)
				{
					newy = from->y - seedistance + std::abs(i);
					newx = from->x + i;
					oldy = from->y + seedistance+1 - std::abs(i);
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
				for (int i = -seedistance; i <= seedistance; ++i)
				{
					newx = from->x + seedistance - std::abs(i);
					newy = from->y + i;
					oldx = from->x - seedistance+1 + std::abs(i);
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
				for (int i = -seedistance; i <= seedistance; ++i)
				{
					newy = from->y + seedistance - std::abs(i);
					newx = from->x + i;
					oldy = from->y - seedistance+1 + std::abs(i);
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
				for (int i = -seedistance; i <= seedistance; ++i)
				{
					newx = from->x - seedistance + std::abs(i);
					newy = from->y + i;
					oldx = from->x + seedistance+1 - std::abs(i);
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

	UTIL_FOREACH(this->items, checkitem)
	{
		switch (direction)
		{
			case DIRECTION_UP:
				for (int i = -seedistance; i <= seedistance; ++i)
				{
					newy = from->y - seedistance + std::abs(i);
					newx = from->x + i;
					oldy = from->y + seedistance+1 - std::abs(i);
					oldx = from->x + i;

					if (checkitem.x == newx && checkitem.y == newy)
					{
						newitems.push_back(checkitem);
					}
				}
				break;

			case DIRECTION_RIGHT:
				for (int i = -seedistance; i <= seedistance; ++i)
				{
					newx = from->x + seedistance - std::abs(i);
					newy = from->y + i;
					oldx = from->x - seedistance+1 + std::abs(i);
					oldy = from->y + i;

					if (checkitem.x == newx && checkitem.y == newy)
					{
						newitems.push_back(checkitem);
					}
				}
				break;

			case DIRECTION_DOWN:
				for (int i = -seedistance; i <= seedistance; ++i)
				{
					newy = from->y + seedistance - std::abs(i);
					newx = from->x + i;
					oldy = from->y - seedistance+1 + std::abs(i);
					oldx = from->x + i;

					if (checkitem.x == newx && checkitem.y == newy)
					{
						newitems.push_back(checkitem);
					}
				}
				break;

			case DIRECTION_LEFT:
				for (int i = -seedistance; i <= seedistance; ++i)
				{
					newx = from->x - seedistance + std::abs(i);
					newy = from->y + i;
					oldx = from->x + seedistance+1 - std::abs(i);
					oldy = from->y + i;

					if (checkitem.x == newx && checkitem.y == newy)
					{
						newitems.push_back(checkitem);
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
	builder.AddShort(eoserv_items->Get(from->paperdoll[Character::Boots])->dollgraphic);
	builder.AddShort(0); // ??
	builder.AddShort(0); // ??
	builder.AddShort(0); // ??
	builder.AddShort(eoserv_items->Get(from->paperdoll[Character::Armor])->dollgraphic);
	builder.AddShort(0); // ??
	builder.AddShort(eoserv_items->Get(from->paperdoll[Character::Hat])->dollgraphic);
	builder.AddShort(eoserv_items->Get(from->paperdoll[Character::Shield])->dollgraphic);
	builder.AddShort(eoserv_items->Get(from->paperdoll[Character::Weapon])->dollgraphic);
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
		rbuilder.AddShort(eoserv_items->Get(character->paperdoll[Character::Boots])->dollgraphic);
		rbuilder.AddShort(0); // ??
		rbuilder.AddShort(0); // ??
		rbuilder.AddShort(0); // ??
		rbuilder.AddShort(eoserv_items->Get(character->paperdoll[Character::Armor])->dollgraphic);
		rbuilder.AddShort(0); // ??
		rbuilder.AddShort(eoserv_items->Get(character->paperdoll[Character::Hat])->dollgraphic);
		rbuilder.AddShort(eoserv_items->Get(character->paperdoll[Character::Shield])->dollgraphic);
		rbuilder.AddShort(eoserv_items->Get(character->paperdoll[Character::Weapon])->dollgraphic);
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

	builder.Reset();

	builder.SetID(PACKET_WALK, PACKET_REPLY);
	builder.AddByte(255);
	builder.AddByte(255);
	UTIL_FOREACH(newitems, item)
	{
		builder.AddShort(item.uid);
		builder.AddShort(item.id);
		builder.AddChar(item.x);
		builder.AddChar(item.y);
		builder.AddThree(item.amount);
	}
	from->player->client->SendBuilder(builder);
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

	from->sitting = SIT_FLOOR;

	builder.SetID(PACKET_SIT, PACKET_PLAYER);
	builder.AddShort(from->player->id);
	builder.AddChar(from->x);
	builder.AddChar(from->y);
	builder.AddChar(from->direction);
	builder.AddChar(0); // ?

	UTIL_FOREACH(characters, character)
	{
		if (character == from || !from->InRange(character))
		{
			continue;
		}

		character->player->client->SendBuilder(builder);
	}
}

void Map::Stand(Character *from)
{
	PacketBuilder builder;

	from->sitting = SIT_STAND;

	builder.SetID(PACKET_SIT, PACKET_REMOVE);
	builder.AddShort(from->player->id);
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

Map_Item *Map::AddItem(int id, int amount, int x, int y, Character *from)
{
	Map_Item newitem = {GenerateItemID(), id, amount, x, y};

	PacketBuilder builder;
	builder.SetID(PACKET_ITEM, PACKET_ADD);
	builder.AddShort(id);
	builder.AddShort(newitem.uid);
	builder.AddThree(amount);
	builder.AddChar(x);
	builder.AddChar(y);

	UTIL_FOREACH(this->characters, character)
	{
		if ((from && character == from) || !character->InRange(newitem))
		{
			continue;
		}
		character->player->client->SendBuilder(builder);
	}

	this->items.push_back(newitem);
	return &this->items.back();
}

void Map::DelItem(int uid, Character *from)
{
	UTIL_FOREACH(this->items, item)
	{
		if (item.uid == uid)
		{
			this->items.erase(util_it);
			PacketBuilder builder;
			builder.SetID(PACKET_ITEM, PACKET_REMOVE);
			builder.AddShort(uid);
			UTIL_FOREACH(this->characters, character)
			{
				if ((from && character == from) || !character->InRange(item))
				{
					continue;
				}
				character->player->client->SendBuilder(builder);
			}
			break;
		}
	}
}
