
CLIENT_F_FUNC(Jukebox)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_OPEN: // Opened the jukebox listing
		{

		}
		break;

		case PACKET_MSG: // Requested a song
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
