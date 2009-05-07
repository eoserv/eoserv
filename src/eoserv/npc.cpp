
NPC::NPC(Map *map, int id, int x, int y, int distance, int spawn_time, int index)
{
	this->map = map;
	this->index = index;
	this->id = id;
	this->spawn_x = this->x = x;
	this->spawn_y = this->y = y;

	this->distance = distance;
	this->spawn_time = spawn_time;

	this->data = eoserv_npcs->Get(id);
	if (distance >= 7)
	{
		this->alive = true;
		this->attack = false;
		this->direction = spawn_time;
	}
	else
	{
		this->alive = false;
		this->min_x = x - distance;
		this->max_x = x + distance;
		this->min_y = y - distance;
		this->max_y = y + distance;
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
			std::fprintf(stderr, "Warning: An NPC on map %i is being placed by linear scan of spawn area\n", this->map->id);
			for (this->x = min_x; this->x <= max_x; ++this->x)
			{
				for (this->y = min_x; this->y <= max_x; ++this->y)
				{
					if (this->map->Walkable(this->x, this->y, true))
					{
						found = true;
						break;
					}
				}
			}
		}

		if (!found)
		{
			std::fputs("Error: NPC couldn't spawn anywhere!\n", stderr);
		}
	}

	this->alive = true;
	//this->hp = this->data->hp;
}
