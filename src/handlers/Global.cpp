
CLIENT_F_FUNC(Global)
{
// Used to track if a user is watching global or not, to decide whether or not
// to send them messages.
// Larger servers may wish to enable this feature to save bandwidth.

	PacketBuilder reply;

	switch (action)
	{
		case PACKET_OPEN: // User has opened the global tab

			break;

		case PACKET_CLOSE: // User has closed the global tab

			break;

		default:
			return false;
	}

	return true;
}
