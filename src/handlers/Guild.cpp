
CLIENT_F_FUNC(Guild)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_OPEN: // Talked to a guild NPC
		// In order to prevent going out of sync with walking, requests will be stored
		// in the same queue.

			break;

		case PACKET_TELL: // Requested member list of a guild

			break;

		case PACKET_REPORT: // Requested information on a guild

			break;

		default:
			return false;
	}

	return true;
}
