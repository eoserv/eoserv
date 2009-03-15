
CLIENT_F_FUNC(Character)
{
	PacketBuilder reply;

	if (!this->player) return true;

	switch (action)
	{
		case PACKET_REQUEST: // Request to create a new character
		{
			if (!this->player) return false;

			reply.SetID(PACKET_CHARACTER, PACKET_REPLY);
			reply.AddShort(1000); // CreateID?
			reply.AddString("OK");
			CLIENT_SEND(reply);
		}
		break;

		case PACKET_CREATE: // Create a character
		{
			if (!this->player) return false;

			reader.GetShort(); // CreateID?

			int gender = reader.GetShort();
			int hairstyle = reader.GetShort();
			int haircolor = reader.GetShort();
			int race = reader.GetShort();
			reader.GetByte();
			std::string name = reader.GetBreakString();
			std::transform(name.begin(), name.end(), name.begin(), static_cast<int(*)(int)>(std::tolower));

			if (gender < 0 || gender > 1) return false;
			if (hairstyle < 1 || hairstyle > 20) return false;
			if (haircolor < 0 || haircolor > 9) return false;
			if (race < 0 || race > 3) return false;

			if (!Character::ValidName(name))
			{
				reply.SetID(PACKET_CHARACTER, PACKET_REPLY);
				reply.AddShort(4); // Reply code
				CLIENT_SEND(reply);
				return true;
			}

			/*if (Character::Exists(name))
			{
				reply.SetID(PACKET_CHARACTER, PACKET_REPLY);
				reply.AddShort(1); // Reply code
				CLIENT_SEND(reply);
				return true;
			}*/

			this->player->AddCharacter(name, gender, hairstyle, haircolor, race);

			reply.SetID(PACKET_CHARACTER, PACKET_REPLY);
			reply.AddShort(5); // Reply code
			// 1 = Name already exists
			// 2,3 = Can't have more than 3 characters
			// 4 = Name not approved
			// 5 = OK (character list follows)
			reply.AddChar(this->player->characters.size()); // Number of characters
			reply.AddByte(1); // ??
			reply.AddByte(255);
			UTIL_FOREACH(this->player->characters, character)
			{
				reply.AddBreakString(character->name); // Character name
				reply.AddInt(character->id); // character id
				reply.AddChar(character->level); // level
				reply.AddChar(character->gender); // sex (0 = female, 1 = male)
				reply.AddChar(character->hairstyle); // hair style
				reply.AddChar(character->haircolor); // hair color
				reply.AddChar(character->race); // race (0 = white, 1 = azn, 2 = nigger, 3 = orc, 4 = skeleton, 5 = panda)
				reply.AddChar(character->admin); // admin level
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

		case PACKET_REMOVE: // Delete a character from an account
		{
			if (!this->player) return false;

			/*int deleteid = */reader.GetShort();
			unsigned int charid = reader.GetInt();

			bool yourchar = false;
			typeof(this->player->characters.begin()) char_it;

			UTIL_FOREACH(this->player->characters, character)
			{
				if (character->id == charid)
				{
					Character::Delete(character->name);
					char_it = util_it;
					yourchar = true;
					break;
				}
			}

			if (!yourchar)
			{
				return false;
			}

			this->player->characters.erase(char_it);

			reply.SetID(PACKET_CHARACTER, PACKET_REPLY);
			reply.AddShort(6); // Reply code
			// 6 = OK (character list follows)
			reply.AddChar(this->player->characters.size()); // Number of characters
			reply.AddByte(1); // ??
			reply.AddByte(255);
			UTIL_FOREACH(this->player->characters, character)
			{
				reply.AddBreakString(character->name); // Character name
				reply.AddInt(character->id); // character id
				reply.AddChar(character->level); // level
				reply.AddChar(character->gender); // sex (0 = female, 1 = male)
				reply.AddChar(character->hairstyle); // hair style
				reply.AddChar(character->haircolor); // hair color
				reply.AddChar(character->race); // race (0 = white, 1 = azn, 2 = nigger, 3 = orc, 4 = skeleton, 5 = panda)
				reply.AddChar(character->admin); // admin level
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

		case PACKET_TAKE: // Request to delete a character from an account
		{
			unsigned int charid = reader.GetInt();

			bool yourchar = false;

			UTIL_FOREACH(this->player->characters, character)
			{
				if (character->id == charid)
				{
					yourchar = true;
					break;
				}
			}

			if (!yourchar)
			{
				return false;
			}

			reply.SetID(PACKET_CHARACTER, PACKET_PLAYER);
			reply.AddShort(1000); // Delete req ID
			reply.AddInt(charid);
			CLIENT_SEND(reply);
		}
		break;

		default:
			return false;
	}

	return true;
}
