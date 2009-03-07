
CLIENT_F_FUNC(Door)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_OPEN: // User opening a door
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
