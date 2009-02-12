
CLIENT_F_FUNC(Walk)
{
// All walk actions are stored in a queue for later handling
// The queue is read as fast as a normal client should be sending walk packets.
// NOTE: This is the stored in the same queue as PACKET_ATTACK

	PacketBuilder reply;

	switch (action)
	{
		case PACKET_PLAYER: // Player walking

			break;

		case PACKET_MOVESPEC: // Player walking (ghost)
		// Ghost walking is only available 5 seconds after not moving
		// This is the shortest amount of time possible with the EO client.

			break;

		default:
			return false;
	}

	return true;
}
