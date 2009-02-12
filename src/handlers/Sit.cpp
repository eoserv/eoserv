
CLIENT_F_FUNC(Sit)
{
// In order to prevent going out of sync with walking, requests will be stored
// in the same queue.

	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Player sitting down

			break;

		default:
			return false;
	}

	return true;
}
