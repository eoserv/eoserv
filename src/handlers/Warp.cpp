
CLIENT_F_FUNC(Warp)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_ACCEPT: // Player accepting a warp request from the server
		// This is generally ignored, and warping is performed automatically

			break;

		default:
			return false;
	}

	return true;
}
