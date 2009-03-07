
CLIENT_F_FUNC(Shop)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_BUY: // Purchasing an item from a store
		{

		}
		break;

		case PACKET_SELL: // Selling an item to a store
		{

		}
		break;

		case PACKET_OPEN: // Talking to a store NPC
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
