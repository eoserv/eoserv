
NPC::NPC(Map *map, short id, unsigned char x, unsigned char y, unsigned char distance, short spawn_time, unsigned char index)
{
	this->map = map;
	this->index = index;
	this->id = id;
	this->spawn_x = this->x = x;
	this->spawn_y = this->y = y;

	this->alive = false;
	this->attack = false;

	this->distance = distance;
	this->spawn_time = spawn_time;

	this->data = eoserv_npcs->Get(id);
	if (distance >= 7)
	{
		this->direction = spawn_time;
	}
	else
	{
		this->min_x = x - distance;
		if (this->min_x > x) min_x = 0;
		this->max_x = x + distance;
		if (this->max_x < x) max_x = 255;
		this->min_y = y - distance;
		if (this->min_y > y) min_y = 0;
		this->max_y = y + distance;
		if (this->max_y < y) max_y = 255;
		this->tries = distance + 3;
		this->tries = this->tries * this->tries * this->tries;
	}
}

void NPC::Spawn()
{
	if (this->distance == 0)
	{
		this->x = this->spawn_x;
		this->y = this->spawn_y;
		this->direction = util::rand(0,3);
	}
	else if (this->distance < 7)
	{
		bool found = false;
		for (int i = 0; i < this->tries; ++i)
		{
			this->x = util::rand(this->min_x, this->max_x);
			this->y = util::rand(this->min_y, this->max_y);

			if (this->map->Walkable(this->x, this->y, true))
			{
				this->direction = util::rand(0,3);
				found = true;
				break;
			}
		}

		if (!found)
		{
			std::fprintf(stderr, "Warning: An NPC on map %i at %i,%i is being placed by linear scan of spawn area after %i attempts\n", this->map->id, this->spawn_x, this->spawn_y, this->tries);
			for (this->x = min_x; this->x <= max_x; ++this->x)
			{
				for (this->y = min_x; this->y <= max_x; ++this->y)
				{
					if (this->map->Walkable(this->x, this->y, true))
					{
						std::fprintf(stderr, "Placed at valid location: %i,%i\n", this->x, this->y);
						found = true;
						goto end_linear_scan;
					}
				}
			}
		}
		end_linear_scan:

		if (!found)
		{
			std::fputs("Error: NPC couldn't spawn anywhere valid!\n", stderr);
		}
	}

	this->alive = true;
	this->hp = this->data->hp;

	PacketBuilder builder(PACKET_APPEAR, PACKET_REPLY);
	builder.AddChar(0);
	builder.AddByte(255);
	builder.AddChar(this->index);
	builder.AddShort(this->id);
	builder.AddChar(this->x);
	builder.AddChar(this->y);
	builder.AddChar(this->direction);

	UTIL_LIST_FOREACH_ALL(this->map->characters, Character *, character)
	{
		if (character->InRange(this))
		{
			character->player->client->SendBuilder(builder);
		}
	}
}

void NPC::Damage(Character *from, int amount)
{
	PacketBuilder builder;

	this->hp -= amount;

	if (this->hp > 0)
	{
		builder.SetID(PACKET_NPC, PACKET_REPLY);
		builder.AddShort(from->id);
		builder.AddChar(1); // ?
		builder.AddShort(this->index);
		builder.AddThree(amount); // damage
		builder.AddShort(int((double(this->hp) / this->data->hp)*100)); // % HP remaining
		builder.AddChar(1); // ?

		UTIL_LIST_FOREACH_ALL(this->map->characters, Character *, character)
		{
			if (character->InRange(this))
			{
				character->player->client->SendBuilder(builder);
			}
		}
	}
	else
	{
		this->alive = false;
		this->dead_since = Timer::GetTime();

		bool level_up = false;

		from->exp += this->data->exp;

		if (from->exp >= the_world->exp_table[from->level+1])
		{
			level_up = true;
			++from->level;
			from->statpoints += static_cast<int>(eoserv_config["StatPerLevel"]);
			from->skillpoints += static_cast<int>(eoserv_config["SkillPerLevel"]);
			from->CalculateStats();
		}

		if (level_up)
		{
			builder.SetID(PACKET_NPC, PACKET_ACCEPT);
		}
		else
		{
			builder.SetID(PACKET_NPC, PACKET_SPEC);
		}

		UTIL_LIST_FOREACH_ALL(this->map->characters, Character *, character)
		{
			if (character->InRange(this))
			{
				builder.Reset();
				builder.AddShort(from->id);
				builder.AddChar(0); // ?
				builder.AddShort(this->index);
				builder.AddShort(0); // ?
				builder.AddShort(0); // dropped item ID
				builder.AddChar(this->x);
				builder.AddChar(this->y);
				builder.AddInt(0); // items dropped
				builder.AddThree(amount);

				if (!level_up || (level_up && character == from))
				{
					builder.AddInt(character->exp);
				}

				if (level_up && character == from)
				{
					builder.AddChar(character->level);
					builder.AddShort(character->statpoints);
					builder.AddShort(character->skillpoints);
					builder.AddShort(character->maxhp);
					builder.AddShort(character->maxtp);
					builder.AddShort(character->maxsp);
				}

				character->player->client->SendBuilder(builder);
			}
		}
	}
}
