
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

/*

1236994521 [RACTION ] someone-appeared2 (3_29) - ☻ hawt ;♥7■♣■↔■♦♥SA☺
♦☺F■F■$■$■.■☺■☺■☺■→■☺■1■☺■♀■☺☺ 
1236994521 [RECV >>E] 57 3 29 2 255 104 97 119 116 255 59 3 55 254 5 254 29 254
4 3 83 65 85 8 1 10 4 1 70 254 70 254 36 254 36 254 46 254 1 254 1 254 1 254 26
254 1 254 49 254 1 254 12 254 1 1 255 126
1236994521 [RECV >>D] 57 254 131 127 157 129 130 129 127 126 232 140 225 126 247
 129 244 126 127 177 187 126 131 129 183 126 126 154 133 126 126 129 157 126 126
 129 132 126 131 129 211 126 193 174 213 126 136 164 129 126 138 164 132 126 129
 198 198 126

1236994623 [RACTION ] someone-disappeared (4_22) - ;♥
1236994623 [RECV >>E] 5 4 22 59 3 126
1236994623 [RECV >>D] 5 254 132 131 150 187

*/
