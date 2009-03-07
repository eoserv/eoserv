
CLIENT_F_FUNC(Sit)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Player sitting down
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
