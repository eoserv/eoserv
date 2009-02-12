
CLIENT_F_FUNC(Character)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REMOVE: // Delete a character from an account

			break;

		case PACKET_CREATE: // (unsure) Create a character

			break;

		default:
			return false;
	}

	return true;
}
