
CLIENT_F_FUNC(Attack)
{
// All attack actions are stored in a queue for later handling
// The queue is read as fast as a normal client should be sending walk packets.
// NOTE: This is the stored in the same queue as PACKET_WALK

	PacketBuilder reply;

	switch (action)
	{
		case PACKET_USE: // Player attacking

			break;

		default:
			return false;
	}

	return true;
}
