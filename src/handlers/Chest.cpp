
CLIENT_F_FUNC(Chest)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_ADD: // Placing an item in a chest
		{

		}
		break;

		case PACKET_TAKE: // Taking an item from a chest
		{

		}
		break;

		case PACKET_OPEN: // Opening a chest
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
