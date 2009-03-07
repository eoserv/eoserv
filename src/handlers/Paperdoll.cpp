
CLIENT_F_FUNC(Paperdoll)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Request for currently equipped items
		{

		}
		break;

		case PACKET_REMOVE: // Unequipping an item
		{

		}
		break;

		case PACKET_ADD: // Equipping an item
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
