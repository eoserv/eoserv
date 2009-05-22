
Character::Character(std::string name)
{
	Database_Result res = eoserv_db.Query("SELECT `name`, `title`, `home`, `partner`, `admin`, `class`, `gender`, `race`, `hairstyle`, `haircolor`, `map`,"
	"`x`, `y`, `direction`, `spawnmap`, `spawnx`, `spawny`, `level`, `exp`, `hp`, `tp`, `str`, `int`, `wis`, `agi`, `con`, `cha`, `statpoints`, `skillpoints`, "
	"`karma`, `sitting`, `bankmax`, `goldbank`, `usage`, `inventory`, `bank`, `paperdoll`, `spells`, `guild`, `guild_rank` FROM `characters` WHERE `name` = '$'", name.c_str());
	std::map<std::string, util::variant> row = res.front();

	this->login_time = std::time(0);

	this->online = true;
	this->id = the_world->GenerateCharacterID();

	this->admin = static_cast<AdminLevel>(static_cast<int>(row["admin"]));
	this->name = static_cast<std::string>(row["name"]);
	this->title = static_cast<std::string>(row["title"]);
	this->home = static_cast<std::string>(row["home"]);
	this->partner = static_cast<std::string>(row["partner"]);

	this->clas = static_cast<int>(row["class"]);
	this->gender = static_cast<Gender>(static_cast<int>(row["gender"]));
	this->race = static_cast<Skin>(static_cast<int>(row["race"]));
	this->hairstyle = static_cast<int>(row["hairstyle"]);
	this->haircolor = static_cast<int>(row["haircolor"]);

	this->mapid = static_cast<int>(row["map"]);
	this->x = static_cast<int>(row["x"]);
	this->y = static_cast<int>(row["y"]);
	this->direction = static_cast<int>(row["direction"]);

	this->spawnmap = static_cast<int>(row["spawnmap"]);
	this->spawnx = static_cast<int>(row["spawnx"]);
	this->spawny = static_cast<int>(row["spawny"]);

	this->level = static_cast<int>(row["level"]);
	this->exp = static_cast<int>(row["exp"]);

	this->hp = static_cast<int>(row["hp"]);
	this->tp = static_cast<int>(row["tp"]);

	this->str = static_cast<int>(row["str"]);
	this->intl = static_cast<int>(row["int"]);
	this->wis = static_cast<int>(row["wis"]);
	this->agi = static_cast<int>(row["agi"]);
	this->con = static_cast<int>(row["con"]);
	this->cha = static_cast<int>(row["cha"]);
	this->statpoints = static_cast<int>(row["statpoints"]);
	this->skillpoints = static_cast<int>(row["skillpoints"]);
	this->karma = static_cast<int>(row["karma"]);

	this->weight = 0;
	this->maxweight = 0;

	this->maxhp = 0;
	this->maxtp = 0;
	this->maxsp = 0;

	this->mindam = 0;
	this->maxdam = 0;

	this->accuracy = 0;
	this->evade = 0;
	this->armor = 0;

	this->trading = false;
	this->trade_partner = 0;
	this->trade_agree = false;

	this->warp_anim = 0;

	this->sitting = static_cast<SitAction>(static_cast<int>(row["sitting"]));

	this->bankmax = static_cast<int>(row["bankmax"]);
	this->goldbank = static_cast<int>(row["goldbank"]);

	this->usage = static_cast<int>(row["usage"]);

	this->inventory = ItemUnserialize(row["inventory"]);
	this->paperdoll = DollUnserialize(row["paperdoll"]);

	this->player = 0;
	this->guild = 0;
	this->guild_tag = util::trim(static_cast<std::string>(row["guild"]));
	this->guild_rank = static_cast<int>(row["guild_rank"]);
	this->party = 0;
	this->map = the_world->maps[0];
}

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

	for (std::size_t i = 0; i < name.length(); ++i)
	{
		if (name[i] < 'a' || name[i] > 'z')
		{
			return false;
		}
	}

	return true;
}

bool Character::Exists(std::string name)
{
	Database_Result res = eoserv_db.Query("SELECT 1 FROM `characters` WHERE `name` = '$'", name.c_str());
	return !res.empty();
}

Character *Character::Create(Player *player, std::string name, int gender, int hairstyle, int haircolor, int race)
{
	char buffer[1024];
	std::string startmapinfo;
	std::string startmapval;
	std::string spawnmapinfo;
	std::string spawnmapval;

	if (static_cast<int>(eoserv_config["StartMap"]))
	{
		startmapinfo = ", `map`, `x`, `y`";
		snprintf(buffer, 1024, ",%i,%i,%i", static_cast<int>(eoserv_config["StartMap"]), static_cast<int>(eoserv_config["StartX"]), static_cast<int>(eoserv_config["StartY"]));
		startmapval = buffer;
	}

	if (static_cast<int>(eoserv_config["SpawnMap"]))
	{
		spawnmapinfo = ", `spawnmap`, `spawnx`, `spawny`";
		snprintf(buffer, 1024, ",%i,%i,%i", static_cast<int>(eoserv_config["SpawnMap"]), static_cast<int>(eoserv_config["SpawnX"]), static_cast<int>(eoserv_config["SpawnY"]));
		spawnmapval = buffer;
	}

	eoserv_db.Query("INSERT INTO `characters` (`name`, `account`, `gender`, `hairstyle`, `haircolor`, `race`, `inventory`, `bank`, `paperdoll`, `spells`@@) VALUES ('$','$',#,#,#,#,'$','','$','$'@@)",
		startmapinfo.c_str(), spawnmapinfo.c_str(), name.c_str(), player->username.c_str(), gender, hairstyle, haircolor, race,
		static_cast<std::string>(eoserv_config["StartItems"]).c_str(), static_cast<std::string>(gender?eoserv_config["StartEquipMale"]:eoserv_config["StartEquipFemale"]).c_str(),
		static_cast<std::string>(eoserv_config["StartSpells"]).c_str(), startmapval.c_str(), spawnmapval.c_str());
	return new Character(name);
}

void Character::Delete(std::string name)
{
	eoserv_db.Query("DELETE FROM `characters` WHERE name = '$'", name.c_str());
}

void Character::Msg(Character *from, std::string message)
{
	PacketBuilder builder;

	builder.SetID(PACKET_TALK, PACKET_TELL);
	builder.AddBreakString(from->name);
	builder.AddBreakString(message);
	this->player->client->SendBuilder(builder);
}

bool Character::Walk(Direction direction)
{
	return this->map->Walk(this, direction);
}

bool Character::AdminWalk(Direction direction)
{
	return this->map->Walk(this, direction, true);
}

void Character::Attack(Direction direction)
{
	this->map->Attack(this, direction);
}

void Character::Sit(SitAction sit_type)
{
	this->map->Sit(this, sit_type);
}

void Character::Stand()
{
	this->map->Stand(this);
}

void Character::Emote(enum Emote emote, bool relay)
{
	this->map->Emote(this, emote, relay);
}

int Character::HasItem(short item)
{
	UTIL_LIST_FOREACH_ALL(this->inventory, Character_Item, it)
	{
		if (it.id == item)
		{
			if (this->trading)
			{
				UTIL_LIST_FOREACH_ALL(this->trade_inventory, Character_Item, tit)
				{
					if (tit.id == item)
					{
						return std::max(it.amount - tit.amount, 0);
					}
				}

				return it.amount;
			}
			else
			{
				return it.amount;
			}
		}
	}

	return 0;
}

bool Character::AddItem(short item, int amount)
{
	Character_Item newitem;

	if (amount <= 0)
	{
		return false;
	}

	if (item <= 0 || static_cast<std::size_t>(item) >= eoserv_items->data.size())
	{
		return false;
	}

	UTIL_LIST_IFOREACH_ALL(this->inventory, Character_Item, it)
	{
		if (it->id == item)
		{
			if (it->amount + amount < 0)
			{
				return false;
			}
			it->amount += amount;

			it->amount = std::min<int>(it->amount, eoserv_config["MaxItem"]);

			this->CalculateStats();
			return true;
		}
	}

	newitem.id = item;
	newitem.amount = amount;

	this->inventory.push_back(newitem);
	this->CalculateStats();
	return true;
}

bool Character::DelItem(short item, int amount)
{
	if (amount <= 0)
	{
		return false;
	}

	UTIL_LIST_IFOREACH_ALL(this->inventory, Character_Item, it)
	{
		if (it->id == item)
		{
			if (it->amount - amount > it->amount || it->amount - amount <= 0)
			{
				this->inventory.erase(it);
			}
			else
			{
				it->amount -= amount;
			}

			this->CalculateStats();
			return true;
		}
	}

	return false;
}

bool Character::AddTradeItem(short item, int amount)
{
	Character_Item newitem;

	if (amount <= 0)
	{
		return false;
	}

	if (item <= 0 || static_cast<std::size_t>(item) >= eoserv_items->data.size())
	{
		return false;
	}

	int hasitem = this->HasItem(item);

	if (hasitem - amount < 0)
	{
		return false;
	}

	UTIL_LIST_IFOREACH_ALL(this->trade_inventory, Character_Item, it)
	{
		if (it->id == item)
		{
			it->amount += amount;
			return true;
		}
	}

	newitem.id = item;
	newitem.amount = amount;

	this->trade_inventory.push_back(newitem);
	return true;
}

bool Character::DelTradeItem(short item)
{
	UTIL_LIST_IFOREACH_ALL(this->trade_inventory, Character_Item, it)
	{
		if (it->id == item)
		{
			this->trade_inventory.erase(it);
			return true;
		}
	}

	return false;
}

bool Character::Unequip(short item, unsigned char subloc)
{
	if (item == 0)
	{
		return false;
	}

	for (std::size_t i = 0; i < this->paperdoll.size(); ++i)
	{
		if (this->paperdoll[i] == item)
		{
			if (((i == Character::Ring2 || i == Character::Armlet2 || i == Character::Bracer2) ? 1 : 0) == subloc)
			{
				this->paperdoll[i] = 0;
				this->AddItem(item, 1);
				this->CalculateStats();
				return true;
			}
		}
	}

	return false;
}

bool Character::Equip(short item, unsigned char subloc)
{
	if (!this->HasItem(item))
	{
		return false;
	}

	switch (eoserv_items->Get(item)->type)
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
			if (eoserv_items->Get(item)->gender != this->gender)
			{
				return false;
			}
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
			if (subloc == 0)
			{
				if (this->paperdoll[Character::Ring1] != 0)
				{
					return false;
				}
				this->paperdoll[Character::Ring1] = item;
				this->DelItem(item, 1);
				break;
			}
			else
			{
				if (this->paperdoll[Character::Ring2] != 0)
				{
					return false;
				}
				this->paperdoll[Character::Ring2] = item;
				this->DelItem(item, 1);
				break;
			}

		case EIF::Armlet:
			if (subloc == 0)
			{
				if (this->paperdoll[Character::Armlet1] != 0)
				{
					return false;
				}
				this->paperdoll[Character::Armlet1] = item;
				this->DelItem(item, 1);
				break;
			}
			else
			{
				if (this->paperdoll[Character::Armlet2] != 0)
				{
					return false;
				}
				this->paperdoll[Character::Armlet2] = item;
				this->DelItem(item, 1);
				break;
			}

		case EIF::Bracer:
			if (subloc == 0)
			{
				if (this->paperdoll[Character::Bracer1] != 0)
				{
					return false;
				}
				this->paperdoll[Character::Bracer1] = item;
				this->DelItem(item, 1);
				break;
			}
			else
			{
				if (this->paperdoll[Character::Bracer2] != 0)
				{
					return false;
				}
				this->paperdoll[Character::Bracer2] = item;
				this->DelItem(item, 1);
				break;
			}

		default:
			return false;
	}

	this->CalculateStats();

	return true;
}

bool Character::InRange(unsigned char x, unsigned char y)
{
	unsigned char xdistance = std::abs(this->x - x);
	unsigned char ydistance = std::abs(this->y - y);
	return (xdistance + ydistance) <= 11;
}

bool Character::InRange(Character *other)
{
	return this->InRange(other->x, other->y);
}

bool Character::InRange(NPC *other)
{
	return this->InRange(other->x, other->y);
}

bool Character::InRange(Map_Item other)
{
	return this->InRange(other.x, other.y);
}

void Character::Warp(short map, unsigned char x, unsigned char y, WarpAnimation animation)
{
	if (map <= 0 || static_cast<std::size_t>(map) > the_world->maps.size() || !the_world->maps[map]->exists)
	{
		return;
	}

	PacketBuilder builder;
	builder.SetID(PACKET_WARP, PACKET_REQUEST);

	if (this->player->character->mapid == map)
	{
		builder.AddChar(WARP_LOCAL);
		builder.AddShort(map);
		builder.AddChar(x);
		builder.AddChar(y);
	}
	else
	{
		builder.AddChar(WARP_SWITCH);
		builder.AddShort(map);
		builder.AddByte(the_world->maps[map]->rid[0]);
		builder.AddByte(the_world->maps[map]->rid[1]);
		builder.AddByte(the_world->maps[map]->rid[2]);
		builder.AddByte(the_world->maps[map]->rid[3]);
		builder.AddThree(the_world->maps[map]->filesize);
		builder.AddChar(0); // ?
		builder.AddChar(0); // ?
	}

	this->map->Leave(this, animation);
	this->map = the_world->maps[map];
	this->mapid = map;
	this->x = x;
	this->y = y;
	this->sitting = SIT_STAND;
	this->map->Enter(this, animation);

	this->warp_anim = animation;

	this->player->client->SendBuilder(builder);
}

void Character::Refresh()
{
	PacketBuilder builder;

	std::vector<Character *> updatecharacters;
	std::vector<NPC *> updatenpcs;
	std::vector<Map_Item> updateitems;

	UTIL_VECTOR_FOREACH_ALL(this->map->characters, Character *, character)
	{
		if (this->InRange(character))
		{
			updatecharacters.push_back(character);
		}
	}

	UTIL_VECTOR_FOREACH_ALL(this->map->npcs, NPC *, npc)
	{
		if (this->InRange(npc))
		{
			updatenpcs.push_back(npc);
		}
	}

	UTIL_VECTOR_FOREACH_ALL(this->map->items, Map_Item, item)
	{
		if (this->InRange(item))
		{
			updateitems.push_back(item);
		}
	}

	builder.SetID(PACKET_REFRESH, PACKET_REPLY);
	builder.AddChar(updatecharacters.size()); // Number of players
	builder.AddByte(255);

	UTIL_VECTOR_FOREACH_ALL(updatecharacters, Character *, character)
	{
		builder.AddBreakString(character->name);
		builder.AddShort(character->player->id);
		builder.AddShort(character->mapid);
		builder.AddShort(character->x);
		builder.AddShort(character->y);
		builder.AddChar(character->direction);
		builder.AddChar(6); // ?
		builder.AddString(character->PaddedGuildTag());
		builder.AddChar(character->level);
		builder.AddChar(character->gender);
		builder.AddChar(character->hairstyle);
		builder.AddChar(character->haircolor);
		builder.AddChar(character->race);
		builder.AddShort(character->maxhp);
		builder.AddShort(character->hp);
		builder.AddShort(character->maxtp);
		builder.AddShort(character->tp);
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
	}

	UTIL_VECTOR_FOREACH_ALL(updatenpcs, NPC *, npc)
	{
		if (npc->alive)
		{
			builder.AddChar(npc->index);
			builder.AddShort(npc->data->id);
			builder.AddChar(npc->x);
			builder.AddChar(npc->y);
			builder.AddChar(npc->direction);
		}
	}

	builder.AddByte(255);

	UTIL_VECTOR_FOREACH_ALL(updateitems, Map_Item, item)
	{
		builder.AddShort(item.uid);
		builder.AddShort(item.id);
		builder.AddChar(item.x);
		builder.AddChar(item.y);
		builder.AddThree(item.amount);
	}

	this->player->client->SendBuilder(builder);
}

std::string Character::PaddedGuildTag()
{
	std::string tag;

	if (static_cast<int>(eoserv_config["ShowLevel"]))
	{
		tag = util::to_string(this->level);
		if (tag.length() < 3)
		{
			tag.insert(tag.begin(), 'L');
		}
	}
	else
	{
		tag = this->guild_tag;
	}

	for (std::size_t i = tag.length(); i < 3; ++i)
	{
		tag.push_back(' ');
	}

	return tag;
}

int Character::Usage()
{
	return this->usage + (std::time(0) - this->login_time) / 60;
}

// TODO: calculate equipment bonuses, check formulas
void Character::CalculateStats()
{
	short calcstr = this->str;
	short calcintl = this->intl;
	short calcwis = this->wis;
	short calcagi = this->agi;
	short calccon = this->con;
	short calccha = this->cha;
	this->maxweight = 70 + this->str;

	if (this->maxweight < 70 || this->maxweight > 250)
	{
		this->maxweight = 250;
	}

	this->weight = 0;
	this->maxhp = 0;
	this->maxtp = 0;
	this->mindam = 0;
	this->maxdam = 0;
	this->accuracy = 0;
	this->evade = 0;
	this->armor = 0;
	this->maxsp = 0;
	UTIL_LIST_FOREACH_ALL(this->inventory, Character_Item, item)
	{
		this->weight += eoserv_items->Get(item.id)->weight * item.amount;
	}
	UTIL_ARRAY_FOREACH_ALL(this->paperdoll, int, 15, i)
	{
		if (i)
		{
			EIF_Data *item = eoserv_items->Get(i);
			this->weight += item->weight;
			this->maxhp += item->hp;
			this->maxtp += item->tp;
			this->mindam += item->mindam;
			this->maxdam += item->maxdam;
			this->accuracy += item->accuracy;
			this->evade += item->evade;
			this->armor += item->armor;
			calcstr += item->str;
			calcintl += item->intl;
			calcwis += item->wis;
			calcagi += item->agi;
			calccon += item->con;
			calccha += item->cha;
		}
	}
	this->maxhp += 10 + calccon*3;
	this->maxtp += 10 + calcwis*3;
	this->mindam += 1 + calcstr/2;
	this->maxdam += 2 + calcstr/2;
	this->accuracy += 0 + calcagi/2;
	this->evade += 0 + calcagi/2;
	this->armor += 0 + calccon/2;
	this->maxsp += std::min(20 + this->level*2, 100);
}

void Character::Save()
{

#ifdef DEBUG
	std::printf("Saving character '%s' (session lasted %i minutes)\n", this->name.c_str(), int(std::time(0) - this->login_time) / 60);
#endif // DEBUG
	eoserv_db.Query("UPDATE `characters` SET `title` = '$', `home` = '$', `partner` = '$', `class` = #, `gender` = #, `race` = #, "
	                "`hairstyle` = #, `haircolor` = #, `map` = #, `x` = #, `y` = #, `direction` = #, `level` = #, `exp` = #, `hp` = #, `tp` = #, "
	                "`str` = #, `int` = #, `wis` = #, `agi` = #, `con` = #, `cha` = #, `statpoints` = #, `skillpoints` = #, `karma` = #, `sitting` = #, "
	                "`bankmax` = #, `goldbank` = #, `usage` = #, `inventory` = '$', `bank` = '$', `paperdoll` = '$', "
	                "`spells` = '$', `guild` = '$', guild_rank = # WHERE `name` = '$'",
                    this->title.c_str(), this->home.c_str(), this->partner.c_str(), this->clas, this->gender, this->race,
	                this->hairstyle, this->haircolor, this->mapid, this->x, this->y, this->direction, this->level, this->exp, this->hp, this->tp,
	                this->str, this->intl, this->wis, this->agi, this->con, this->cha, this->statpoints, this->skillpoints, this->karma, this->sitting,
	                this->bankmax, this->goldbank, this->Usage(), ItemSerialize(this->inventory).c_str(), "", DollSerialize(this->paperdoll).c_str(),
	                "", this->guild_tag.c_str(), this->guild_rank, this->name.c_str());
}

Character::~Character()
{
	if (this->trading)
	{
		PacketBuilder builder(PACKET_TRADE, PACKET_CLOSE);
		builder.AddShort(this->id);
		this->trade_partner->player->client->SendBuilder(builder);

		this->player->client->state = EOClient::Playing;
		this->trading = false;
		this->trade_inventory.clear();
		this->trade_agree = false;

		this->trade_partner->player->client->state = EOClient::Playing;
		this->trade_partner->trading = false;
		this->trade_partner->trade_inventory.clear();
		this->trade_agree = false;

		this->trade_partner->trade_partner = 0;
		this->trade_partner = 0;
	}

	UTIL_LIST_FOREACH_ALL(this->unregister_npc, NPC *, npc)
	{
		UTIL_LIST_IFOREACH_ALL(npc->damagelist, NPC_Opponent, checkopp)
		{
			if (checkopp->attacker == this)
			{
				npc->totaldamage -= checkopp->damage;
				npc->damagelist.erase(checkopp);
				break;
			}
		}
	}

	if (this->player)
	{
		the_world->Logout(this);
	}

	this->Save();
}
