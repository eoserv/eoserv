
CLIENT_F_FUNC(Bank)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_OPEN: // Talked to a banker NPC
		// In order to prevent going out of sync with walking, requests will be stored
		// in the same queue.

			break;

		case PACKET_ADD: // Depositing gold

			break;

		case PACKET_TAKE: // Withdrawing gold

			break;

		default:
			return false;
	}

	return true;
}
