
CLIENT_F_FUNC(Walk)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_ADMIN: // Player walking (admin)
		{
			if (this->state < EOClient::Playing) return false;

			if (this->player->character->admin < ADMIN_GUARDIAN)
			{
				return false;
			}
		}
		// no break

		case PACKET_PLAYER: // Player walking (normal)
		case PACKET_SPEC: // Player walking (ghost)
		{
			if (this->state < EOClient::Playing) return false;
			if (!act) CLIENT_QUEUE_ACTION(0.5);

			int direction = reader.GetChar();
			/*int timestamp = */reader.GetThree();
			int x = reader.GetChar();
			int y = reader.GetChar();

			if (this->player->character->sitting != SIT_STAND)
			{
				return true;
			}

			if (direction >= 0 && direction <= 3)
			{
				if (action == PACKET_ADMIN)
				{
					this->player->character->AdminWalk(direction);
				}
				else
				{
					if (!this->player->character->Walk(direction))
					{
						return true;
					}
				}
			}

			if (this->player->character->x != x || this->player->character->y != y)
			{
				this->player->character->Refresh();
			}

		}
		break;

		default:
			return false;
	}

	return true;
}
