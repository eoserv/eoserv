
CLIENT_F_FUNC(Chair)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Player sitting on a chair
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
