
CLIENT_F_FUNC(Character)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Request to create a new character
		{
			if (this->state != EOClient::LoggedIn) return false;

			reply.SetID(PACKET_CHARACTER, PACKET_REPLY);
			reply.AddShort(1000); // CreateID?
			reply.AddString("OK");
			CLIENT_SEND(reply);
		}
		break;

		case PACKET_CREATE: // Create a character
		{
			if (this->state != EOClient::LoggedIn) return false;

			reader.GetShort(); // CreateID?

			int gender = reader.GetShort();
			int hairstyle = reader.GetShort();
			int haircolor = reader.GetShort();
			int race = reader.GetShort();
			reader.GetByte();
			std::string name = reader.GetBreakString();
			util::lowercase(name);

			if (gender < 0 || gender > 1) return false;
			if (hairstyle < static_cast<int>(eoserv_config["CreateMinHairStyle"]) || hairstyle > static_cast<int>(eoserv_config["CreateMaxHairStyle"])) return false;
			if (haircolor < static_cast<int>(eoserv_config["CreateMinHairColor"]) || haircolor > static_cast<int>(eoserv_config["CreateMaxHairColor"])) return false;
			if (race < static_cast<int>(eoserv_config["CreateMinSkin"]) || race > static_cast<int>(eoserv_config["CreateMaxSkin"])) return false;

			reply.SetID(PACKET_CHARACTER, PACKET_REPLY);

			if (this->player->characters.size() >= static_cast<std::size_t>(static_cast<int>(eoserv_config["MaxCharacters"])))
			{
				reply.AddShort(CHARACTER_FULL); // Reply code
			}
			else if (!Character::ValidName(name))
			{
				reply.AddShort(CHARACTER_NOT_APPROVED); // Reply code
			}
			else if (Character::Exists(name))
			{
				reply.AddShort(CHARACTER_EXISTS); // Reply code
			}
			else
			{
				this->player->AddCharacter(name, gender, hairstyle, haircolor, race);

				reply.AddShort(CHARACTER_OK);
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
			}

			CLIENT_SEND(reply);

		}
		break;

		case PACKET_REMOVE: // Delete a character from an account
		{
			if (this->state != EOClient::LoggedIn) return false;

			/*int deleteid = */reader.GetShort();
			unsigned int charid = reader.GetInt();

			bool yourchar = false;
			std::list<Character *>::iterator char_it;

			UTIL_LIST_IFOREACH(this->player->characters.begin(), this->player->characters.end(), Character *, character)
			{
				if ((*character)->id == charid)
				{
					Character::Delete((*character)->name);
					char_it = character;
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
			reply.AddShort(CHARACTER_DELETED); // Reply code
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

		case PACKET_TAKE: // Request to delete a character from an account
		{
			if (this->state != EOClient::LoggedIn) return false;

			unsigned int charid = reader.GetInt();

			bool yourchar = false;

			UTIL_LIST_FOREACH_ALL(this->player->characters, Character *, character)
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
