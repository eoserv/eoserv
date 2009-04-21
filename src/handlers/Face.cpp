
CLIENT_F_FUNC(Face)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_PLAYER: // Player changing direction
		{
			if (!this->player || !this->player->character || this->player->character->modal) return false;

			int direction = reader.GetChar();

			if (this->player->character->sitting != SIT_STAND)
			{
				return true;
			}

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
