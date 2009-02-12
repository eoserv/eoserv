
CLIENT_F_FUNC(Jukebox)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_OPEN: // Opened the jukebox listing
		// In order to prevent going out of sync with walking, requests will be stored
		// in the same queue.

			break;

		case PACKET_MSG: // Requested a song

			break;

		default:
			return false;
	}

	return true;
}
