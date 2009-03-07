
CLIENT_F_FUNC(Party)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // (unsure) Invite/Join request
		{

		}
		break;

		case PACKET_ACCEPT: // (unsure) Accept invite/join request
		{

		}
		break;

		case PACKET_REMOVE: // (unsure) Remove a player from a party
		{

		}
		break;

		case PACKET_TAKE: // Requested list of party members
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
