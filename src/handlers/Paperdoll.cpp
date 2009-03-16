
CLIENT_F_FUNC(Paperdoll)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Request for currently equipped items
		{
			if (!this->player || !this->player->character) return false;

			unsigned int id = reader.GetShort();
			Character *character = this->player->character;

			UTIL_FOREACH(this->player->character->map->characters, checkcharacter)
			{
				if (checkcharacter->player->id == id)
				{
					character = checkcharacter;
					break;
				}
			}

			reply.SetID(PACKET_PAPERDOLL, PACKET_REPLY);
			reply.AddBreakString(character->name);
			reply.AddBreakString(character->home);
			reply.AddBreakString(character->partner);
			reply.AddBreakString(character->title);
			reply.AddBreakString("Guild Name");
			reply.AddBreakString("Guild Rank");
			reply.AddShort(character->player->id); // ?
			reply.AddChar(character->clas);
			reply.AddChar(character->gender);
			reply.AddChar(0); // admin/party flag?
			for (int i = 0; i < 15; ++i)
			{
				reply.AddShort(character->paperdoll[i]);
			}
			CLIENT_SEND(reply);
		}
		break;

		case PACKET_REMOVE: // Unequipping an item
		{
			if (!this->player || !this->player->character) return false;

			int itemid = reader.GetShort();
			int subloc = reader.GetChar(); // Used for double slot items (rings etc)

			if (this->player->character->Unequip(itemid, subloc))
			{
				reply.SetID(PACKET_PAPERDOLL, PACKET_REMOVE);
				reply.AddShort(this->player->id);
				reply.AddChar(1); // ?
				reply.AddChar(subloc);
				reply.AddShort(eoserv_items->GetDollGraphic(this->player->character->paperdoll[Character::Boots]));
				reply.AddShort(eoserv_items->GetDollGraphic(this->player->character->paperdoll[Character::Armor]));
				reply.AddShort(eoserv_items->GetDollGraphic(this->player->character->paperdoll[Character::Hat]));
				reply.AddShort(eoserv_items->GetDollGraphic(this->player->character->paperdoll[Character::Weapon]));
				reply.AddShort(eoserv_items->GetDollGraphic(this->player->character->paperdoll[Character::Shield]));
				reply.AddShort(itemid);
				reply.AddChar(0); // ?
				reply.AddShort(this->player->character->maxhp);
				reply.AddShort(this->player->character->maxtp);
				reply.AddShort(this->player->character->str);
				reply.AddShort(this->player->character->intl);
				reply.AddShort(this->player->character->wis);
				reply.AddShort(this->player->character->agi);
				reply.AddShort(this->player->character->con);
				reply.AddShort(this->player->character->cha);
				reply.AddShort(this->player->character->mindam);
				reply.AddShort(this->player->character->maxdam);
				reply.AddShort(this->player->character->accuracy);
				reply.AddShort(this->player->character->evade);
				reply.AddShort(this->player->character->armor);
				CLIENT_SEND(reply);
			}
			// TODO: Only send this if they change a viewable item

			PacketBuilder builder;
			builder.SetID(PACKET_CLOTHES, PACKET_AGREE);
			builder.AddShort(this->player->id);
			builder.AddChar(1); // ?
			builder.AddChar(subloc);
			builder.AddShort(eoserv_items->GetDollGraphic(this->player->character->paperdoll[Character::Boots]));
			builder.AddShort(eoserv_items->GetDollGraphic(this->player->character->paperdoll[Character::Armor]));
			builder.AddShort(eoserv_items->GetDollGraphic(this->player->character->paperdoll[Character::Hat]));
			builder.AddShort(eoserv_items->GetDollGraphic(this->player->character->paperdoll[Character::Weapon]));
			builder.AddShort(eoserv_items->GetDollGraphic(this->player->character->paperdoll[Character::Shield]));

			UTIL_FOREACH(this->player->character->map->characters, character)
			{
				if (character == this->player->character || !this->player->character->InRange(character))
				{
					continue;
				}
				character->player->client->SendBuilder(builder);
			}
		}
		break;

		case PACKET_ADD: // Equipping an item
		{
			if (!this->player || !this->player->character) return false;

			int itemid = reader.GetShort();
			int subloc = reader.GetChar(); // Used for double slot items (rings etc)

			// TODO: Find out if we can handle equipping items when we have more than 16.7m of them better

			if (this->player->character->Equip(itemid, subloc))
			{
				reply.SetID(PACKET_PAPERDOLL, PACKET_AGREE);
				reply.AddShort(this->player->id);
				reply.AddChar(1); // ?
				reply.AddChar(subloc);
				reply.AddShort(eoserv_items->GetDollGraphic(this->player->character->paperdoll[Character::Boots]));
				reply.AddShort(eoserv_items->GetDollGraphic(this->player->character->paperdoll[Character::Armor]));
				reply.AddShort(eoserv_items->GetDollGraphic(this->player->character->paperdoll[Character::Hat]));
				reply.AddShort(eoserv_items->GetDollGraphic(this->player->character->paperdoll[Character::Weapon]));
				reply.AddShort(eoserv_items->GetDollGraphic(this->player->character->paperdoll[Character::Shield]));
				reply.AddShort(itemid);
				reply.AddThree(this->player->character->HasItem(itemid));
				reply.AddChar(0); // ?
				reply.AddShort(this->player->character->maxhp);
				reply.AddShort(this->player->character->maxtp);
				reply.AddShort(this->player->character->str);
				reply.AddShort(this->player->character->intl);
				reply.AddShort(this->player->character->wis);
				reply.AddShort(this->player->character->agi);
				reply.AddShort(this->player->character->con);
				reply.AddShort(this->player->character->cha);
				reply.AddShort(this->player->character->mindam);
				reply.AddShort(this->player->character->maxdam);
				reply.AddShort(this->player->character->accuracy);
				reply.AddShort(this->player->character->evade);
				reply.AddShort(this->player->character->armor);
				CLIENT_SEND(reply);
			}

			// TODO: Only send this if they change a viewable item

			PacketBuilder builder;
			builder.SetID(PACKET_CLOTHES, PACKET_AGREE);
			builder.AddShort(this->player->id);
			builder.AddChar(1); // ?
			builder.AddChar(subloc);
			builder.AddShort(eoserv_items->GetDollGraphic(this->player->character->paperdoll[Character::Boots]));
			builder.AddShort(eoserv_items->GetDollGraphic(this->player->character->paperdoll[Character::Armor]));
			builder.AddShort(eoserv_items->GetDollGraphic(this->player->character->paperdoll[Character::Hat]));
			builder.AddShort(eoserv_items->GetDollGraphic(this->player->character->paperdoll[Character::Weapon]));
			builder.AddShort(eoserv_items->GetDollGraphic(this->player->character->paperdoll[Character::Shield]));

			UTIL_FOREACH(this->player->character->map->characters, character)
			{
				if (character == this->player->character || !this->player->character->InRange(character))
				{
					continue;
				}
				character->player->client->SendBuilder(builder);
			}
		}
		break;

		default:
			return false;
	}

	return true;
}
