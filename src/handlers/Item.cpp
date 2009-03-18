
CLIENT_F_FUNC(Item)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_USE: // Player using an item
		{
			if (!this->player || !this->player->character) return false;
		}
		break;

		case PACKET_DROP: // Drop an item on the ground
		{
			if (!this->player || !this->player->character) return false;

			int id = reader.GetShort();
			int amount;
			if (reader.Length() == 8)
			{
				amount = reader.GetThree();
			}
			else if (reader.Length() == 9)
			{
				amount = reader.GetInt();
			}
			int x = reader.GetByte(); // ?
			int y = reader.GetByte(); // ?

			if (x == 255 && y == 255)
			{
				x = this->player->character->x;
				y = this->player->character->y;
			}
			else
			{
				x = PacketProcessor::Number(x);
				y = PacketProcessor::Number(y);
			}

			// TODO: Range check
			if (this->player->character->HasItem(id) >= amount)
			{
				Map_Item item = this->player->character->map->AddItem(id, amount, x, y, this->player->character);
				this->player->character->DelItem(id, amount);

				reply.SetID(PACKET_ITEM, PACKET_DROP);
				reply.AddShort(id);
				reply.AddThree(amount);
				reply.AddInt(this->player->character->HasItem(id));
				reply.AddShort(item.uid);
				reply.AddChar(x);
				reply.AddChar(y);
				reply.AddChar(this->player->character->weight);
				reply.AddChar(this->player->character->maxweight);
				CLIENT_SEND(reply);
			}
		}
		break;

		case PACKET_JUNK: // Destroying an item
		{
			if (!this->player || !this->player->character) return false;

			int id = reader.GetShort();
			int amount = reader.GetInt();

			this->player->character->DelItem(id, amount);

			reply.SetID(PACKET_ITEM, PACKET_JUNK);
			reply.AddShort(id);
			reply.AddThree(amount); // Overflows, does it matter?
			reply.AddInt(this->player->character->HasItem(id));
			reply.AddChar(this->player->character->weight);
			reply.AddChar(this->player->character->maxweight);
			CLIENT_SEND(reply);
		}
		break;

		case PACKET_GET: // Retrieve an item from the ground
		{
			if (!this->player || !this->player->character) return false;

			int uid = reader.GetShort();

			// TODO: Range check

			UTIL_FOREACH(this->player->character->map->items, item)
			{
				if (item.uid == uid)
				{
					this->player->character->AddItem(item.id, item.amount);
					this->player->character->map->DelItem(uid, this->player->character);

					reply.SetID(PACKET_ITEM, PACKET_GET);
					reply.AddShort(uid);
					reply.AddShort(item.id);
					reply.AddThree(item.amount);
					reply.AddChar(this->player->character->weight);
					reply.AddChar(this->player->character->maxweight);
					CLIENT_SEND(reply);
					break;
				}
			}

		}
		break;

		default:
			return false;
	}

	return true;
}
