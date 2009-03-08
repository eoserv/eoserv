
CLIENT_F_FUNC(Character)
{
	PacketBuilder reply;

	if (!this->player) return true;

	switch (action)
	{
		case PACKET_REQUEST: // Request to create a new character
		{
			reply.SetID(PACKET_CHARACTER, PACKET_REPLY);
			reply.AddShort(1000); // CreateID?
			reply.AddString("OK");
			CLIENT_SEND(reply);
		}
		break;

		case PACKET_CREATE: // Create a character
		{
			bool valid = true;

			reader.GetByte(); // Ordering byte

			reader.GetShort(); // CreateID?

			int gender = reader.GetShort();
			int hairstyle = reader.GetShort();
			int haircolor = reader.GetShort();
			int race = reader.GetShort();
			reader.GetByte();
			std::string name = reader.GetBreakString();
			std::transform(name.begin(), name.end(), name.begin(), std::tolower);

			if (gender < 0 || gender > 1) valid = false;
			if (hairstyle < 1 || hairstyle > 20) valid = false;
			if (haircolor < 0 || haircolor > 9) valid = false;
			if (race < 0 || race > 3) valid = false;

			if (!Character::ValidName(name))
			{
				reply.SetID(PACKET_CHARACTER, PACKET_REPLY);
				reply.AddShort(5); // Reply code
				CLIENT_SEND(reply);
				return true;
			}

			this->player->AddCharacter(name, gender, hairstyle, haircolor, race);

			reply.SetID(PACKET_CHARACTER, PACKET_REPLY);
			reply.AddShort(5); // Reply code
			// 1 = Name already exists
			// 2,3 = Can't have more than 3 characters
			// 4 = Name not approved
			// 5 = OK (character list follows)
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

		case PACKET_REMOVE: // Delete a character from an account
		{
			reply.SetID(PACKET_CHARACTER, PACKET_REPLY);
			reply.AddShort(3); // Reply code (see Login.cpp)
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

		case PACKET_TAKE: // Request to delete a character from an account
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
