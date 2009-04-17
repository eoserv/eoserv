
CLIENT_F_FUNC(Refresh)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // User requesting data of all objects in their location
		{
			if (!this->player || !this->player->character) return false;

			std::list<Character *> updatecharacters;
			std::list<Map_Item> updateitems;
			UTIL_LIST_FOREACH_ALL(this->player->character->map->characters, Character *, character)
			{
				if (this->player->character->InRange(character))
				{
					updatecharacters.push_back(character);
				}
			}
			UTIL_LIST_FOREACH_ALL(this->player->character->map->items, Map_Item, item)
			{
				if (this->player->character->InRange(item))
				{
					updateitems.push_back(item);
				}
			}
			reply.SetID(PACKET_REFRESH, PACKET_REPLY);
			reply.AddChar(updatecharacters.size()); // Number of players
			reply.AddByte(255);
			UTIL_LIST_FOREACH_ALL(updatecharacters, Character *, character)
			{
				reply.AddBreakString(character->name);
				reply.AddShort(character->player->id);
				reply.AddShort(character->mapid);
				reply.AddShort(character->x);
				reply.AddShort(character->y);
				reply.AddChar(character->direction);
				reply.AddChar(6); // ?
				reply.AddString(character->PaddedGuildTag());
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
				reply.AddShort(eoserv_items->Get(character->paperdoll[Character::Boots])->dollgraphic);
				reply.AddShort(0); // ??
				reply.AddShort(0); // ??
				reply.AddShort(0); // ??
				reply.AddShort(eoserv_items->Get(character->paperdoll[Character::Armor])->dollgraphic);
				reply.AddShort(0); // ??
				reply.AddShort(eoserv_items->Get(character->paperdoll[Character::Hat])->dollgraphic);
				reply.AddShort(eoserv_items->Get(character->paperdoll[Character::Shield])->dollgraphic);
				reply.AddShort(eoserv_items->Get(character->paperdoll[Character::Weapon])->dollgraphic);
				reply.AddChar(character->sitting);
				reply.AddChar(0); // visible
				reply.AddByte(255);
			}
			reply.AddByte(255);
			UTIL_LIST_FOREACH_ALL(updateitems, Map_Item, item)
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
