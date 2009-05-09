
CLIENT_F_FUNC(Emote)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REPORT: // Player sending an emote
		{
			if (!this->player || !this->player->character) return false;

			int emote = reader.GetChar();

			if ((emote >= 0 && emote <= 10) || emote == 14)
			{
				this->player->character->Emote(emote, false);
			}
		}
		break;

		default:
			return false;
	}

	return true;
}
