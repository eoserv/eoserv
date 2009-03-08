
CLIENT_F_FUNC(Sit)
{
	PacketBuilder reply;

	return true; // TODO: Fix sitting

	switch (action)
	{
		case PACKET_REQUEST: // Player sitting down
		{
			if (!this->player || !this->player->character || !this->player->character->map) return false;

			reader.GetByte(); // Ordering byte
			int action = reader.GetChar();

			if (action == SIT_SITTING)
			{
				puts("SIT");
				int x = reader.GetChar();
				int y = reader.GetChar();

				this->player->character->Sit();

				if (this->player->character->x != x || this->player->character->y != y)
				{
					// TODO: refresh out-of-sync players
				}
			}
			else
			{
				puts("STAND");
				this->player->character->Stand();
			}
		}
		break;

		default:
			return false;
	}

	return true;
}
