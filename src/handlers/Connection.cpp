
CLIENT_F_FUNC(Connection)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_ACCEPT: // Reply after PACKET_INIT transfer is complete
		{

		}
		break;

		case PACKET_NET: // Response to a PACKET_PING from the server
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
