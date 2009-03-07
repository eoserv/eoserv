
CLIENT_F_FUNC(Ping)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_NET: // User sending a ping request (#ping)
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
