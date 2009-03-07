
CLIENT_F_FUNC(AdminInteract)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_TELL: // "Talk to admin" message
		{

		}
		break;

		case PACKET_REPORT: // User report
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
