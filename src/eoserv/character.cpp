
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

	this->sitting = static_cast<int>(row["sitting"]);

	this->bankmax = static_cast<int>(row["bankmax"]);
	this->goldbank = static_cast<int>(row["goldbank"]);

	this->usage = static_cast<int>(row["usage"]);

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
	                "`bankmax` = #, `goldbank` = #, `usage` = # WHERE `name` = '$'",
                    this->title.c_str(), this->home.c_str(), this->partner.c_str(), this->clas, this->gender, this->race,
	                this->hairstyle, this->haircolor, this->mapid, this->x, this->y, this->direction, this->level, this->exp, this->hp, this->tp,
	                this->str, this->intl, this->wis, this->agi, this->con, this->cha, this->statpoints, this->skillpoints, this->sitting,
	                this->bankmax, this->goldbank, this->usage, this->name.c_str());
}
