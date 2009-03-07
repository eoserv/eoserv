
CLIENT_F_FUNC(Face)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_PLAYER: // Player changing direction
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
