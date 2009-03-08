
CLIENT_F_FUNC(Attack)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_USE: // Player walking
		{
			if (!this->player || !this->player->character || !this->player->character->map) return false;

			reader.GetByte(); // Ordering byte
			int direction = reader.GetChar();
			int timestamp = reader.GetThree();

			if (direction >= 0 && direction <= 3)
			{
				this->player->character->Attack(direction);
			}
		}
		break;

		default:
			return false;
	}

	return true;
}
