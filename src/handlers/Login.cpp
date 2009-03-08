#include <vector>
CLIENT_F_FUNC(Login)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Logging in to an account
		{
			if (this->player) return false;

			reader.GetByte(); // Ordering byte
			std::string username =  reader.GetBreakString();
			std::string password = reader.GetBreakString();

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
			reply.AddChar(this->player->characters.size()); // Number of characters
			reply.AddByte(1); // ??
			reply.AddByte(255); // ??
			UTIL_FOREACH(this->player->characters, character)
			{
				reply.AddBreakString(character->name); // Character name
				reply.AddChar(250); // ??
				reply.AddThree(character->id); // character id
				reply.AddChar(character->level); // level
				reply.AddChar(character->gender); // sex (0 = female, 1 = male)
				reply.AddChar(character->hairstyle); // hair style
				reply.AddChar(character->haircolor); // hair color
				reply.AddChar(character->race); // race (0 = white, 1 = azn, 2 = nigger, 3 = orc, 4 = skeleton, 5 = panda)
				reply.AddChar(character->admin); // admin level
				reply.AddShort(0); // shoes
				reply.AddShort(0); // armor
				reply.AddShort(0); // hat
				reply.AddShort(0); // shield
				reply.AddShort(0); // weapon
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
