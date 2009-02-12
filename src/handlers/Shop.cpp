
CLIENT_F_FUNC(Shop)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_BUY: // Purchasing an item from a store

			break;

		case PACKET_SELL: // Selling an item to a store

			break;

		case PACKET_OPEN: // Talking to a store NPC
		// In order to prevent going out of sync with walking, requests will be stored
		// in the same queue.

			break;

		default:
			return false;
	}

	return true;
}
