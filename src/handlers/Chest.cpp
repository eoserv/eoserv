
CLIENT_F_FUNC(Chest)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_ADD: // Placing an item in a chest

			break;

		case PACKET_TAKE: // Taking an item from a chest

			break;

		case PACKET_OPEN: // Opening a chest
		// In order to prevent going out of sync with walking, requests will be stored
		// in the same queue.

			break;

		default:
			return false;
	}

	return true;
}
