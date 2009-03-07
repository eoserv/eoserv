
CLIENT_F_FUNC(Emote)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REPORT: // Player sending an emote
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
