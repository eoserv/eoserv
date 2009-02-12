
CLIENT_F_FUNC(Item)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_USE: // Player using an item

			break;

		case PACKET_DROP: // Drop an item on the ground
		// In order to prevent going out of sync with walking, requests will be stored
		// in the same queue.

			break;

		case PACKET_JUNK: // Destroying an item

			break;

		case PACKET_GET: // Retrieve an item from the ground
		// In order to prevent going out of sync with walking, requests will be stored
		// in the same queue.


			break;

		default:
			return false;
	}

	return true;
}
