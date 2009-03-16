
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
				reply.AddShort(PACKET_CHARACTER_NOT_APPROVED); // Reply code
				CLIENT_SEND(reply);
				return true;
			}

			if (Character::Exists(name))
			{
				reply.SetID(PACKET_CHARACTER, PACKET_REPLY);
				reply.AddShort(PACKET_CHARACTER_EXISTS); // Reply code
				CLIENT_SEND(reply);
				return true;
			}

			this->player->AddCharacter(name, gender, hairstyle, haircolor, race);

			reply.SetID(PACKET_CHARACTER, PACKET_REPLY);
			reply.AddShort(PACKET_CHARACTER_OK);
			reply.AddChar(this->player->characters.size());
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
				reply.AddByte(255);
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
			reply.AddShort(PACKET_CHARACTER_DELETED); // Reply code
			reply.AddChar(this->player->characters.size());
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
				reply.AddByte(255);
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
				return true;
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
