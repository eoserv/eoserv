
Player::Player(std::string username)
{
	Database_Result res = eoserv_db.Query("SELECT `username`, `password` FROM `accounts` WHERE `username` = '$'", username.c_str());
	if (res.empty())
	{
		return;
	}
	std::map<std::string, util::variant> row = res.front();

	this->login_time = std::time(0);

	this->online = true;
	this->character = 0;

	this->username = static_cast<std::string>(row["username"]);
	this->password = static_cast<std::string>(row["password"]);

	res = eoserv_db.Query("SELECT `name` FROM `characters` WHERE `account` = '$'", username.c_str());

	typedef std::map<std::string, util::variant> Database_Row;
	UTIL_LIST_FOREACH_ALL(res, Database_Row, row)
	{
		Character *newchar = new Character(row["name"]);
		this->characters.push_back(newchar);
		newchar->player = this;
	}

	this->client = 0;
}

bool Player::ValidName(std::string username)
{
	if (username.length() < 4)
	{
		return false;
	}

	if (username.length() > 12)
	{
		return false;
	}

	for (std::size_t i = 0; i < username.length(); ++i)
	{
		if (!((username[i] >= 'a' && username[i] <= 'z') || username[i] == ' ' || (username[i] >= '0' && username[i] <= '9')))
		{
			return false;
		}
	}

	return true;
}

Player *Player::Login(std::string username, std::string password)
{
	password = static_cast<std::string>(eoserv_config["PasswordSalt"]) + username + password;
	sha256(password);
	Database_Result res = eoserv_db.Query("SELECT 1 FROM `accounts` WHERE `username` = '$' AND `password` = '$'", username.c_str(), password.c_str());
	if (res.empty())
	{
		return 0;
	}
	std::map<std::string, util::variant> row = res.front();

	return new Player(username);
}

bool Player::Create(std::string username, std::string password, std::string fullname, std::string location, std::string email, std::string computer, std::string hdid, std::string ip)
{
	password = static_cast<std::string>(eoserv_config["PasswordSalt"]) + username + password;
	sha256(password);
	Database_Result result = eoserv_db.Query("INSERT INTO `accounts` (`username`, `password`, `fullname`, `location`, `email`, `computer`, `hdid`, `regip`, `created`) VALUES ('$','$','$','$','$','$','$','$',#)", username.c_str(), password.c_str(), fullname.c_str(), location.c_str(), email.c_str(), computer.c_str(), hdid.c_str(), ip.c_str(), std::time(0));
	return !result.Error();
}

bool Player::Exists(std::string username)
{
	Database_Result res = eoserv_db.Query("SELECT 1 FROM `accounts` WHERE `username` = '$'", username.c_str());
	return !res.empty();
}

bool Player::AddCharacter(std::string name, int gender, int hairstyle, int haircolor, int race)
{
	if (this->characters.size() > 3)
	{
		return false;
	}

	Character *newchar = Character::Create(this, name, gender, hairstyle, haircolor, race);

	if (!newchar)
	{
		return false;
	}

	newchar->player = this;

	this->characters.push_back(newchar);
	return true;
}

void Player::ChangePass(std::string password)
{
	password = static_cast<std::string>(eoserv_config["PasswordSalt"]) + username + password;
	sha256(password);
	this->password = password;
	Database_Result res = eoserv_db.Query("UPDATE `accounts` SET `password` = '$' WHERE username = '$'", password.c_str(), this->username.c_str());
}

bool Player::Online(std::string username)
{
	if (!Player::ValidName(username))
	{
		return false;
	}

	UTIL_LIST_FOREACH_ALL(the_world->server->clients, EOClient *, connection)
	{
		if (connection->player)
		{
			if (connection->player->username.compare(username) == 0)
			{
				return true;
			}
		}
	}

	return false;
}

Player::~Player()
{
#ifdef DEBUG
	std::printf("Saving player '%s' (session lasted %i minutes)\n", this->username.c_str(), int(std::time(0) - this->login_time) / 60);
#endif // DEBUG
	eoserv_db.Query("UPDATE `accounts` SET `lastused` = #, `lastip` = '$' WHERE username = '$'", std::time(0), static_cast<std::string>(this->client->GetRemoteAddr()).c_str(), this->username.c_str());

	if (this->client)
	{
		// Disconnect the client to make sure this null pointer is never dereferenced
		this->client->Close();
		this->client->player = 0;
	}

	if (this->character)
	{
		delete this->character;
	}
}
