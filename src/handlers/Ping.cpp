
CLIENT_F_FUNC(Ping)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_NET: // User sending a ping request (#ping)
		{
			if (!this->player || !this->player->character) return false;

			int something = reader.GetShort();

			reply.SetID(PACKET_PING, PACKET_NET2);
			reply.AddShort(something);
			CLIENT_SEND(reply);
		}
		break;

		default:
			return false;
	}

	return true;
}
