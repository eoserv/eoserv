
CLIENT_F_FUNC(Trade)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Requesting a trade with another player

			break;

		case PACKET_ACCEPT: // Accepting a trade request

			break;

		case PACKET_REMOVE: // Remove an item from the trade screen

			break;

		case PACKET_AGREE: // Mark your agreeance with the current trade

			break;

		case PACKET_ADD: // Add an item to the trade screen

			break;

		case PACKET_CLOSE: // Cancel the trade

			break;

		default:
			return false;
	}

	return true;
}
