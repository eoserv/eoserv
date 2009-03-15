
CLIENT_F_FUNC(Sit)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Player sitting down
		{
			if (!this->player || !this->player->character) return false;

			int action = reader.GetChar();

			if (action == SIT_SITTING)
			{
				int x = reader.GetChar();
				int y = reader.GetChar();

				reply.SetID(PACKET_SIT, PACKET_PLAYER);
				reply.AddShort(this->player->id);
				reply.AddChar(this->player->character->x);
				reply.AddChar(this->player->character->y);
				reply.AddChar(this->player->character->direction);
				reply.AddChar(0); // ?
				CLIENT_SEND(reply);
				this->player->character->Sit();

				if (this->player->character->x != x || this->player->character->y != y)
				{
					// TODO: refresh out-of-sync players
				}
			}
			else
			{
				reply.SetID(PACKET_SIT, PACKET_PLAYER);
				reply.AddShort(this->player->id);
				reply.AddChar(this->player->character->x);
				reply.AddChar(this->player->character->y);
				CLIENT_SEND(reply);
				this->player->character->Stand();
			}
		}
		break;

		default:
			return false;
	}

	return true;
}
