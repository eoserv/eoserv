
CLIENT_F_FUNC(Walk)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_PLAYER: // Player walking
		{
			if (!this->player || !this->player->character || !this->player->character->map) return false;

			reader.GetByte(); // Ordering byte
			int direction = reader.GetChar();
			int timestamp = reader.GetThree();
			int x = reader.GetChar();
			int y = reader.GetChar();

			if (direction >= 0 && direction <= 3)
			{
				this->player->character->map->Walk(this->player->character, direction);
			}

			if (this->player->character->x != x || this->player->character->y != y)
			{
				// TODO: refresh out-of-sync players
			}
		}
		break;

		case PACKET_MOVESPEC: // Player walking (ghost)
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
