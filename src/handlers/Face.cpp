
CLIENT_F_FUNC(Face)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_PLAYER: // Player changing direction
		{
			if (!this->player || !this->player->character || !this->player->character->map) return false;

			reader.GetByte(); // Ordering byte
			int direction = reader.GetChar();

			if (direction >= 0 && direction <= 3)
			{
				this->player->character->map->Face(this->player->character, direction);
			}
		}
		break;

		default:
			return false;
	}

	return true;
}
