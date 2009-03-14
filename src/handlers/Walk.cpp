
CLIENT_F_FUNC(Walk)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_PLAYER: // Player walking (normal)
		case PACKET_MOVESPEC: // Player walking (ghost)
		case PACKET_MOVEADMIN: // Player walking (admin)
		{
			if (!this->player || !this->player->character || !this->player->character->map) return false;

			int direction = reader.GetChar();
			/*int timestamp = */reader.GetThree();
			int x = reader.GetChar();
			int y = reader.GetChar();

			if (direction >= 0 && direction <= 3)
			{
				this->player->character->Walk(direction);
			}

			reply.SetID(PACKET_WALK, PACKET_REPLY);
			reply.AddByte(255);
			reply.AddByte(255);
			CLIENT_SEND(reply);

			if (this->player->character->x != x || this->player->character->y != y)
			{
				std::list<Character *> updatecharacters;
				UTIL_FOREACH(this->player->character->map->characters, character)
				{
					if (this->player->character->InRange(character))
					{
						updatecharacters.push_back(character);
					}
				}

				PacketBuilder builder;
				builder.SetID(PACKET_REFRESH, PACKET_REPLY);
				builder.AddChar(updatecharacters.size()); // Number of players
				builder.AddByte(255);
				UTIL_FOREACH(updatecharacters, character)
				{
					builder.AddBreakString(character->name);
					builder.AddShort(character->player->id);
					builder.AddShort(character->mapid);
					builder.AddShort(character->x);
					builder.AddShort(character->y);
					builder.AddChar(character->direction);
					builder.AddChar(6); // ?
					builder.AddString("SEX"); // guild tag
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
					builder.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Boots]));
					builder.AddShort(0); // ??
					builder.AddShort(0); // ??
					builder.AddShort(0); // ??
					builder.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Armor]));
					builder.AddShort(0); // ??
					builder.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Hat]));
					builder.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Shield]));
					builder.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Weapon]));
					builder.AddChar(character->sitting);
					builder.AddChar(0); // visible
					builder.AddByte(255);
				}
				CLIENT_SEND(builder);
			}


		}
		break;

		default:
			return false;
	}

	return true;
}
