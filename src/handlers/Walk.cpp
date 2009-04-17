
CLIENT_F_FUNC(Walk)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_MOVEADMIN: // Player walking (admin)
		{
			if (!this->player || !this->player->character) return false;

			if (this->player->character->admin < ADMIN_GUARDIAN)
			{
				return false;
			}
		}
		// no break

		case PACKET_PLAYER: // Player walking (normal)
		case PACKET_MOVESPEC: // Player walking (ghost)
		{
			if (!this->player || !this->player->character) return false;

			int direction = reader.GetChar();
			/*int timestamp = */reader.GetThree();
			int x = reader.GetChar();
			int y = reader.GetChar();

			if (this->player->character->sitting != SIT_STAND)
			{
				return true;
			}

			if (direction >= 0 && direction <= 3)
			{
				if (action == PACKET_MOVEADMIN)
				{
					this->player->character->AdminWalk(direction);
				}
				else
				{
					if (!this->player->character->Walk(direction))
					{
						return true;
					}
				}
			}

			if (this->player->character->x != x || this->player->character->y != y)
			{
				std::list<Character *> updatecharacters;
				UTIL_LIST_FOREACH_ALL(this->player->character->map->characters, Character *, character)
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
				UTIL_LIST_FOREACH_ALL(updatecharacters, Character *, character)
				{
					builder.AddBreakString(character->name);
					builder.AddShort(character->player->id);
					builder.AddShort(character->mapid);
					builder.AddShort(character->x);
					builder.AddShort(character->y);
					builder.AddChar(character->direction);
					builder.AddChar(6); // ?
					builder.AddString(character->PaddedGuildTag());
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
					builder.AddShort(eoserv_items->Get(character->paperdoll[Character::Boots])->dollgraphic);
					builder.AddShort(0); // ??
					builder.AddShort(0); // ??
					builder.AddShort(0); // ??
					builder.AddShort(eoserv_items->Get(character->paperdoll[Character::Armor])->dollgraphic);
					builder.AddShort(0); // ??
					builder.AddShort(eoserv_items->Get(character->paperdoll[Character::Hat])->dollgraphic);
					builder.AddShort(eoserv_items->Get(character->paperdoll[Character::Shield])->dollgraphic);
					builder.AddShort(eoserv_items->Get(character->paperdoll[Character::Weapon])->dollgraphic);
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
