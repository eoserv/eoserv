
CLIENT_F_FUNC(Door)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_OPEN: // User opening a door
		// In order to prevent going out of sync with walking, requests will be stored
		// in the same queue.

			break;

		default:
			return false;
	}

	return true;
}
