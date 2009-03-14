
bool Character::ValidName(std::string name)
{
	if (name.length() < 4)
	{
		return false;
	}

	if (name.length() > 12)
	{
		return false;
	}

	for (const char *p = name.c_str(); *p != '\0'; ++p)
	{
		if (*p < 'a' && *p > 'z')
		{
			return false;
		}
	}

	return true;
}

bool Character::Exists(std::string name)
{
	if (!Character::ValidName(name))
	{
		return false;
	}
	Database_Result res = eoserv_db.Query("SELECT 1 FROM `characters` WHERE `name` = '$'", name.c_str());
	return !res.empty();
}

Character *Character::Create(Player *player, std::string name, int gender, int hairstyle, int haircolor, int race)
{
	if (!Character::ValidName(name))
	{
		return 0;
	}
	eoserv_db.Query("INSERT INTO `characters` (`name`, `account`, `gender`, `hairstyle`, `haircolor`, `race`, `inventory`, `bank`, `paperdoll`, `spells`) VALUES ('$','$',#,#,#,#,'','','','')", name.c_str(), player->username.c_str(), gender, hairstyle, haircolor, race);
	return new Character(name);
}

void Character::Delete(std::string name)
{
	if (!Character::ValidName(name))
	{
		return;
	}
	eoserv_db.Query("DELETE FROM `characters` WHERE name = '$'", name.c_str());
}

Character::Character(std::string name)
{
	static unsigned int id = 1;
	if (!Character::ValidName(name))
	{
		return;
	}
	Database_Result res = eoserv_db.Query("SELECT * FROM `characters` WHERE `name` = '$'", name.c_str());
	std::map<std::string, util::variant> row = res.front();

	this->online = true;
	this->id = id++;

	this->admin = static_cast<int>(row["admin"]);
	this->name = static_cast<std::string>(row["name"]);
	this->title = static_cast<std::string>(row["title"]);
	this->home = static_cast<std::string>(row["home"]);
	this->partner = static_cast<std::string>(row["partner"]);

	this->clas = row["class"];
	this->gender = row["gender"];
	this->race = row["race"];
	this->hairstyle = row["hairstyle"];
	this->haircolor = row["haircolor"];

	this->mapid = row["map"];
	this->x = row["x"];
	this->y = row["y"];
	this->direction = row["direction"];

	this->level = row["level"];
	this->exp = row["exp"];

	this->hp = row["hp"];
	this->tp = row["tp"];

	this->str = row["str"];
	this->intl = row["int"];
	this->wis = row["wis"];
	this->agi = row["agi"];
	this->con = row["con"];
	this->cha = row["cha"];
	this->statpoints = row["statpoints"];
	this->skillpoints = row["skillpoints"];

	this->weight = 0;
	this->maxweight = 250;

	this->maxhp = 100;
	this->maxtp = 100;
	this->maxsp = 100;

	this->mindam = 100;
	this->maxdam = 100;

	this->accuracy = 100;
	this->evade = 100;
	this->armor = 100;

	this->warp_temp = false;

	this->sitting = static_cast<int>(row["sitting"]);

	this->bankmax = static_cast<int>(row["bankmax"]);
	this->goldbank = static_cast<int>(row["goldbank"]);

	this->usage = static_cast<int>(row["usage"]);

	this->inventory = ItemUnserialize(row["inventory"]);
	this->paperdoll = DollUnserialize(row["paperdoll"]);

	this->player = 0;
	this->guild = 0;
	this->party = 0;
	this->map = 0;
}

void Character::Walk(int direction)
{
	this->map->Walk(this, direction);
}

void Character::Attack(int direction)
{
	this->map->Attack(this, direction);
}

void Character::Sit()
{
	this->map->Sit(this);
}

void Character::Stand()
{
	this->map->Stand(this);
}

void Character::Emote(int emote)
{
	this->map->Emote(this, emote);
}

int Character::HasItem(int item)
{
	UTIL_FOREACH(this->inventory, it)
	{
		if (it.first == item)
		{
			return it.second;
		}
	}

	return 0;
}

void Character::AddItem(int item, int amount)
{
	if (amount < 0)
	{
		return;
	}

	UTIL_IFOREACH(this->inventory, it)
	{
		if (it->first == item)
		{
			if (it->second + amount < 0)
			{
				return;
			}
			it->second += amount;

			return;
		}
	}

	this->inventory.push_back(std::make_pair(item, amount));
}

void Character::DelItem(int item, int amount)
{
	if (amount < 0)
	{
		return;
	}

	UTIL_IFOREACH(this->inventory, it)
	{
		if (it->first == item)
		{
			if (it->second - amount > it->second || it->second - amount <= 0)
			{
				this->inventory.erase(it);
			}
			else
			{
				it->second -= amount;
			}

			return;
		}
	}
}

bool Character::Unequip(int item)
{
	for (int i = 0; i < 15; ++i)
	{
		if (this->paperdoll[i] == item)
		{
			this->paperdoll[i] = 0;
			this->AddItem(item, 1);
			return true;
		}
	}

	return false;
}

bool Character::Equip(int item)
{
	if (!this->HasItem(item))
	{
		return false;
	}

	switch (eoserv_items->GetType(item))
	{
		case EIF::Weapon:
			if (this->paperdoll[Character::Weapon] != 0)
			{
				return false;
			}
			this->paperdoll[Character::Weapon] = item;
			this->DelItem(item, 1);
			break;

		case EIF::Shield:
			if (this->paperdoll[Character::Shield] != 0)
			{
				return false;
			}
			this->paperdoll[Character::Shield] = item;
			this->DelItem(item, 1);
			break;

		case EIF::Armor:
			if (this->paperdoll[Character::Armor] != 0)
			{
				return false;
			}
			this->paperdoll[Character::Armor] = item;
			this->DelItem(item, 1);
			break;

		case EIF::Hat:
			if (this->paperdoll[Character::Hat] != 0)
			{
				return false;
			}
			this->paperdoll[Character::Hat] = item;
			this->DelItem(item, 1);
			break;

		case EIF::Boots:
			if (this->paperdoll[Character::Boots] != 0)
			{
				return false;
			}
			this->paperdoll[Character::Boots] = item;
			this->DelItem(item, 1);
			break;

		case EIF::Gloves:
			if (this->paperdoll[Character::Gloves] != 0)
			{
				return false;
			}
			this->paperdoll[Character::Gloves] = item;
			this->DelItem(item, 1);
			break;

		case EIF::Accessory:
			if (this->paperdoll[Character::Accessory] != 0)
			{
				return false;
			}
			this->paperdoll[Character::Accessory] = item;
			this->DelItem(item, 1);
			break;

		case EIF::Belt:
			if (this->paperdoll[Character::Belt] != 0)
			{
				return false;
			}
			this->paperdoll[Character::Belt] = item;
			this->DelItem(item, 1);
			break;

		case EIF::Necklace:
			if (this->paperdoll[Character::Necklace] != 0)
			{
				return false;
			}
			this->paperdoll[Character::Necklace] = item;
			this->DelItem(item, 1);
			break;

		case EIF::Ring:
			if (this->paperdoll[Character::Ring1] != 0)
			{
				if (this->paperdoll[Character::Ring2] != 0)
				{
					return false;
				}
				this->paperdoll[Character::Ring2] = item;
				this->DelItem(item, 1);
				return false;
			}
			this->paperdoll[Character::Ring1] = item;
			this->DelItem(item, 1);
			break;

		case EIF::Armlet:
			if (this->paperdoll[Character::Armlet1] != 0)
			{
				if (this->paperdoll[Character::Armlet2] != 0)
				{
					return false;
				}
				this->paperdoll[Character::Armlet2] = item;
				this->DelItem(item, 1);
			}
			this->paperdoll[Character::Armlet1] = item;
			this->DelItem(item, 1);
			break;

		case EIF::Bracer:
			if (this->paperdoll[Character::Bracer1] != 0)
			{
				if (this->paperdoll[Character::Bracer2] != 0)
				{
					return false;
				}
				this->paperdoll[Character::Bracer2] = item;
				this->DelItem(item, 1);
			}
			this->paperdoll[Character::Bracer1] = item;
			this->DelItem(item, 1);
			break;

		default:
			return false;
	}

	return true;
}

bool Character::InRange(Character *other)
{
	int xdistance = std::abs(this->x - other->x);
	int ydistance = std::abs(this->y - other->y);
	return (xdistance + ydistance) <= 11;
}

void Character::Warp(int map, int x, int y)
{
	PacketBuilder builder;
	builder.SetID(PACKET_WARP, PACKET_REQUEST);

	if (this->player->character->mapid == map)
	{
		builder.AddChar(PACKET_WARP_LOCAL);
		builder.AddShort(map);
		builder.AddChar(x);
		builder.AddChar(y);
	}
	else
	{
		builder.AddChar(PACKET_WARP_SWITCH);
		builder.AddShort(map);
		builder.AddChar(the_world->maps[map]->rid[0]);
		builder.AddChar(the_world->maps[map]->rid[1]);
		builder.AddChar(the_world->maps[map]->rid[2]);
		builder.AddChar(the_world->maps[map]->rid[3]);
		builder.AddChar(0); // ?
		builder.AddShort(0); // ?
		builder.AddChar(0); // ?
		builder.AddChar(0); // ?
	}

	this->map->Leave(this);
	this->map = the_world->maps[map];
	this->mapid = map;
	this->x = x;
	this->y = y;
	this->map->Enter(this);

	this->warp_temp = true;

	this->player->client->SendBuilder(builder);
}

Character::~Character()
{
	if (this->player)
	{
		the_world->Logout(this);
	}

#ifdef DEBUG
	std::printf("Saving character '%s'\n", this->name.c_str());
#endif // DEBUG
	eoserv_db.Query("UPDATE `characters` SET `title` = '$', `home` = '$', `partner` = '$', `class` = #, `gender` = #, `race` = #, "
	                "`hairstyle` = #, `haircolor` = #, `map` = #, `x` = #, `y` = #, `direction` = #, `level` = #, `exp` = #, `hp` = #, `tp` = #, "
	                "`str` = #, `int` = #, `wis` = #, `agi` = #, `con` = #, `cha` = #, `statpoints` = #, `skillpoints` = #, `sitting` = #, "
	                "`bankmax` = #, `goldbank` = #, `usage` = #, `inventory` = '$', `paperdoll` = '$' WHERE `name` = '$'",
                    this->title.c_str(), this->home.c_str(), this->partner.c_str(), this->clas, this->gender, this->race,
	                this->hairstyle, this->haircolor, this->mapid, this->x, this->y, this->direction, this->level, this->exp, this->hp, this->tp,
	                this->str, this->intl, this->wis, this->agi, this->con, this->cha, this->statpoints, this->skillpoints, this->sitting,
	                this->bankmax, this->goldbank, this->usage, ItemSerialize(this->inventory).c_str(), DollSerialize(this->paperdoll).c_str(), this->name.c_str());
}
