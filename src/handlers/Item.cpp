
CLIENT_F_FUNC(Item)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_USE: // Player using an item
		{

		}
		break;

		case PACKET_DROP: // Drop an item on the ground
		{

		}
		break;

		case PACKET_JUNK: // Destroying an item
		{

		}
		break;

		case PACKET_GET: // Retrieve an item from the ground
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
