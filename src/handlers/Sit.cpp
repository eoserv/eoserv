
CLIENT_F_FUNC(Sit)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Player sitting down
		{
			if (!this->player || !this->player->character) return false;

			int action = reader.GetChar();

			if (action == SIT_SITTING && this->player->character->sitting == SIT_STAND)
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
				this->player->character->Sit(SIT_FLOOR);

				if (this->player->character->x != x || this->player->character->y != y)
				{
					// TODO: refresh out-of-sync players
				}
			}
			else if (this->player->character->sitting == SIT_FLOOR)
			{
				reply.SetID(PACKET_SIT, PACKET_CLOSE);
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
