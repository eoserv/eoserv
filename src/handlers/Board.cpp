
CLIENT_F_FUNC(Board)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_CREATE: // Posting to a message board

			break;

		case PACKET_TAKE: // Opening town board
		// In order to prevent going out of sync with walking, requests will be stored
		// in the same queue.

			break;

		case PACKET_OPEN: // Reading a post on a town board

			break;

		default:
			return false;
	}

	return true;
}
