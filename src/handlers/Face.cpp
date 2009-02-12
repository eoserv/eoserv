
CLIENT_F_FUNC(Face)
{
// In order to prevent going out of sync with walking, requests will be stored
// in the same queue.

	PacketBuilder reply;

	switch (action)
	{
		case PACKET_PLAYER: // Player changing direction

			break;

		default:
			return false;
	}

	return true;
}
