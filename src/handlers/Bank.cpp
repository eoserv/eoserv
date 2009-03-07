
CLIENT_F_FUNC(Bank)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_OPEN: // Talked to a banker NPC
		{

		}
		break;

		case PACKET_ADD: // Depositing gold
		{

		}
		break;

		case PACKET_TAKE: // Withdrawing gold
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
