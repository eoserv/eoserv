
CLIENT_F_FUNC(Emote)
{
// In order to prevent going out of sync with walking, requests will be stored
// in the same queue.

	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REPORT: // Player sending an emote

			break;

		default:
			return false;
	}

	return true;
}
