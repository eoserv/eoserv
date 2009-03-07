
CLIENT_F_FUNC(Book)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_LIST: // User requests another's Book (Quest history)
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
