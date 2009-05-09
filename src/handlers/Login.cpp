
CLIENT_F_FUNC(Login)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Logging in to an account
		{
			if (this->player) return false;

			std::string username = reader.GetBreakString();
			std::string password = reader.GetBreakString();

			util::lowercase(username);

			reply.SetID(PACKET_LOGIN, PACKET_REPLY);

			if (Player::Online(username))
			{
				reply.AddShort(LOGIN_LOGGEDIN);
				CLIENT_SEND(reply);
				return true;
			}

			if ((this->player = Player::Login(username, password)) == 0)
			{
				reply.AddShort(LOGIN_WRONG_USERPASS);
				CLIENT_SEND(reply);
				return true;
			}
			this->player->id = this->id;
			this->player->client = this;

			if (the_world->server->UsernameBanned(username))
			{
				this->Close();
				return false;
			}

			reply.AddShort(LOGIN_OK);
			reply.AddChar(this->player->characters.size());
			reply.AddByte(1); // ??
			reply.AddByte(255);
			UTIL_LIST_FOREACH_ALL(this->player->characters, Character *, character)
			{
				reply.AddBreakString(character->name);
				reply.AddInt(character->id);
				reply.AddChar(character->level);
				reply.AddChar(character->gender);
				reply.AddChar(character->hairstyle);
				reply.AddChar(character->haircolor);
				reply.AddChar(character->race);
				reply.AddChar(character->admin);
				reply.AddShort(eoserv_items->Get(character->paperdoll[Character::Boots])->dollgraphic);
				reply.AddShort(eoserv_items->Get(character->paperdoll[Character::Armor])->dollgraphic);
				reply.AddShort(eoserv_items->Get(character->paperdoll[Character::Hat])->dollgraphic);
				reply.AddShort(eoserv_items->Get(character->paperdoll[Character::Shield])->dollgraphic);
				reply.AddShort(eoserv_items->Get(character->paperdoll[Character::Weapon])->dollgraphic);
				reply.AddByte(255);
			}
			CLIENT_SEND(reply);

		}
		break;

		default:
			return false;
	}

	return true;
}
