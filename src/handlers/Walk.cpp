
CLIENT_F_FUNC(Walk)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_PLAYER: // Player walking
		{

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
