
CLIENT_F_FUNC(Paperdoll)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Request for currently equipped items
		{
			if (!this->player || !this->player->character || !this->player->character->map) return false;

			Character *character = this->player->character;

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
			reader.GetChar(); // Unknown

			if (this->player->character->Unequip(itemid))
			{
				reply.SetID(PACKET_PAPERDOLL, PACKET_REMOVE);
				reply.AddChar(27); // ?
				reply.AddChar(13); // ?
				reply.AddChar(1); // ?
				reply.AddChar(0); // ?
				reply.AddShort(0); // ?
				reply.AddShort(25); // ?
				reply.AddShort(48); // ?
				reply.AddShort(11); // ?
				reply.AddShort(0); // ?
				reply.AddShort(itemid);
				reply.AddChar(0); // ?
				reply.AddShort(69); // ?
				reply.AddShort(35); // ?
				reply.AddShort(1); // ?
				reply.AddShort(3); // ?
				reply.AddShort(4); // ?
				reply.AddShort(2); // ?
				reply.AddShort(2); // ?
				reply.AddShort(0); // ?
				reply.AddShort(2); // ?
				reply.AddShort(3); // ?
				reply.AddShort(2); // ?
				reply.AddShort(5); // ?
				reply.AddShort(6); // ?
				CLIENT_SEND(reply);
			}
		}
		break;

		case PACKET_ADD: // Equipping an item
		{
			if (!this->player || !this->player->character || !this->player->character->map) return false;

			int itemid = reader.GetShort();
			reader.GetChar(); // Unknown

			if (this->player->character->Equip(itemid))
			{
				reply.SetID(PACKET_PAPERDOLL, PACKET_AGREE);
				reply.AddChar(27); // ?
				reply.AddChar(13); // ?
				reply.AddChar(1); // ?
				reply.AddChar(0); // ?
				reply.AddShort(45); // ?
				reply.AddShort(25); // ?
				reply.AddShort(48); // ?
				reply.AddShort(11); // ?
				reply.AddShort(0); // ?
				reply.AddShort(itemid);
				reply.AddShort(0); // ?
				reply.AddChar(0); // ?
				reply.AddShort(69); // ?
				reply.AddShort(35); // ?
				reply.AddShort(1); // ?
				reply.AddShort(3); // ?
				reply.AddShort(4); // ?
				reply.AddShort(2); // ?
				reply.AddShort(2); // ?
				reply.AddShort(0); // ?
				reply.AddShort(2); // ?
				reply.AddShort(3); // ?
				reply.AddShort(2); // ?
				reply.AddShort(5); // ?
				reply.AddShort(6); // ?
				CLIENT_SEND(reply);
			}
		}
		break;

		default:
			return false;
	}

	return true;
}
