#include <vector>
CLIENT_F_FUNC(Login)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Logging in to an account
		{
			if (this->player) return false;

			std::string username =  reader.GetBreakString();
			std::string password = reader.GetBreakString();

			std::transform(username.begin(), username.end(), username.begin(), static_cast<int(*)(int)>(std::tolower));

			reply.SetID(PACKET_LOGIN, PACKET_REPLY);

			if (Player::Online(username))
			{
				reply.AddShort(5);
				CLIENT_SEND(reply);
				return true;
			}

			if ((this->player = Player::Login(username, password)) == 0)
			{
				reply.AddShort(2);
				CLIENT_SEND(reply);
				return true;
			}
			this->player->id = this->id;
			this->player->client = this;

			reply.AddShort(3); // Reply code
			// 1 = Wrong user (shouldn't be used)
			// 2 = Wrong user or password
			// 3 = OK (character list follows)
			// 4 = ??
			// 5 = Already logged in
			// 6 = Character deleted / refresh?
			reply.AddChar(this->player->characters.size()); // Number of characters
			reply.AddByte(1); // ??
			reply.AddByte(255);
			UTIL_FOREACH(this->player->characters, character)
			{
				reply.AddBreakString(character->name);
				reply.AddInt(character->id);
				reply.AddChar(character->level);
				reply.AddChar(character->gender);
				reply.AddChar(character->hairstyle);
				reply.AddChar(character->haircolor);
				reply.AddChar(character->race);
				reply.AddChar(character->admin);
				reply.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Boots]));
				reply.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Armor]));
				reply.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Hat]));
				reply.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Shield]));
				reply.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Weapon]));
				reply.AddByte(255); // end of character marker
			}
			CLIENT_SEND(reply);

		}
		break;

		default:
			return false;
	}

	return true;
}
