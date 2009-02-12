
CLIENT_F_FUNC(Chair)
{
// In order to prevent going out of sync with walking, requests will be stored
// in the same queue.

	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Player sitting on a chair

			break;

		default:
			return false;
	}

	return true;
}
