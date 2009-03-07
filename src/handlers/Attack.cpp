
CLIENT_F_FUNC(Attack)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_USE: // Player attacking
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
