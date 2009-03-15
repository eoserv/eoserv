
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
		}
		break;

		default:
			return false;
	}

	return true;
}
