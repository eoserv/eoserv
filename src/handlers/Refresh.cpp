
CLIENT_F_FUNC(Refresh)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // User requesting data of all objects in their location
		{
			if (!this->player || !this->player->character) return false;

			this->player->character->Refresh();
		}
		break;

		default:
			return false;
	}

	return true;
}
