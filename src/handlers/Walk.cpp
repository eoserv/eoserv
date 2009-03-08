
CLIENT_F_FUNC(Walk)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_PLAYER: // Player walking (normal)
		case PACKET_MOVESPEC: // Player walking (ghost)
		case PACKET_MOVEADMIN: // Player walking (admin)
		{
			if (!this->player || !this->player->character || !this->player->character->map) return false;

			reader.GetByte(); // Ordering byte
			int direction = reader.GetChar();
			int timestamp = reader.GetThree();
			int x = reader.GetChar();
			int y = reader.GetChar();

			if (direction >= 0 && direction <= 3)
			{
				this->player->character->Walk(direction);
			}

			if (this->player->character->x != x || this->player->character->y != y)
			{
				// TODO: refresh out-of-sync players
			}
		}
		break;

		default:
			return false;
	}

	return true;
}
