
CLIENT_F_FUNC(Paperdoll)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Request for currently equipped items
		{
			if (!this->player || !this->player->character || !this->player->character->map) return false;

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
			if (!this->player || !this->player->character || !this->player->character->map) return false;

			int itemid = reader.GetShort();
			int subloc = reader.GetChar(); // Used for double slot items (rings etc)

			if (this->player->character->Unequip(itemid, subloc))
			{
				reply.SetID(PACKET_PAPERDOLL, PACKET_REMOVE);
				reply.AddChar(101); // ?
				reply.AddChar(102); // ?
				reply.AddChar(103); // ?
				reply.AddChar(104); // ?
				reply.AddShort(105); // ?
				reply.AddShort(106); // ?
				reply.AddShort(107); // ?
				reply.AddShort(108); // ?
				reply.AddShort(109); // ?
				reply.AddShort(itemid);
				reply.AddChar(110); // ?
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
		}
		break;

		case PACKET_ADD: // Equipping an item
		{
			if (!this->player || !this->player->character || !this->player->character->map) return false;

			int itemid = reader.GetShort();
			int subloc = reader.GetChar(); // Used for double slot items (rings etc)

			if (this->player->character->Equip(itemid, subloc))
			{
				reply.SetID(PACKET_PAPERDOLL, PACKET_AGREE);
				reply.AddChar(120); // ?
				reply.AddChar(121); // ?
				reply.AddChar(122); // ?
				reply.AddChar(subloc); // ?
				reply.AddShort(124); // ?
				reply.AddShort(125); // ?
				reply.AddShort(126); // ?
				reply.AddShort(127); // ?
				reply.AddShort(128); // ?
				reply.AddShort(itemid);
				reply.AddShort(this->player->character->HasItem(itemid));
				reply.AddChar(130); // ?
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
		}
		break;

		default:
			return false;
	}

	return true;
}
