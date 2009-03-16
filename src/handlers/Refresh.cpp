
CLIENT_F_FUNC(Refresh)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // User requesting data of all objects in their location
		{
			std::list<Character *> updatecharacters;
			std::list<Map_Item> updateitems;
			UTIL_FOREACH(this->player->character->map->characters, character)
			{
				if (this->player->character->InRange(character))
				{
					updatecharacters.push_back(character);
				}
			}
			UTIL_FOREACH(this->player->character->map->items, item)
			{
				if (this->player->character->InRange(item))
				{
					updateitems.push_back(item);
				}
			}
			reply.SetID(PACKET_REFRESH, PACKET_REPLY);
			reply.AddChar(updatecharacters.size()); // Number of players
			reply.AddByte(255);
			UTIL_FOREACH(updatecharacters, character)
			{
				reply.AddBreakString(character->name);
				reply.AddShort(character->player->id);
				reply.AddShort(character->mapid);
				reply.AddShort(character->x);
				reply.AddShort(character->y);
				reply.AddChar(character->direction);
				reply.AddChar(6); // ?
				reply.AddString("SEX"); // guild tag
				reply.AddChar(character->level);
				reply.AddChar(character->gender);
				reply.AddChar(character->hairstyle);
				reply.AddChar(character->haircolor);
				reply.AddChar(character->race);
				reply.AddShort(character->maxhp);
				reply.AddShort(character->hp);
				reply.AddShort(character->maxtp);
				reply.AddShort(character->tp);
				// equipment
				reply.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Boots]));
				reply.AddShort(0); // ??
				reply.AddShort(0); // ??
				reply.AddShort(0); // ??
				reply.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Armor]));
				reply.AddShort(0); // ??
				reply.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Hat]));
				reply.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Shield]));
				reply.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Weapon]));
				reply.AddChar(character->sitting);
				reply.AddChar(0); // visible
				reply.AddByte(255);
			}
			CLIENT_SEND(reply);
			reply.AddByte(255);
			UTIL_FOREACH(updateitems, item)
			{
				reply.AddShort(item.uid);
				reply.AddShort(item.id);
				reply.AddChar(item.x);
				reply.AddChar(item.y);
				reply.AddThree(item.amount);
			}
			CLIENT_SEND(reply);
		}
		break;

		default:
			return false;
	}

	return true;
}
