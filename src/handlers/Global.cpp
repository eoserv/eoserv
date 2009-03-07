
CLIENT_F_FUNC(Global)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_OPEN: // User has opened the global tab
		{

		}
		break;

		case PACKET_CLOSE: // User has closed the global tab
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
