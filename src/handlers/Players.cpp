
CLIENT_F_FUNC(Players)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Requested a list of online players
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
