
CLIENT_F_FUNC(Players)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Requested a list of online players
		{
			int replycode = 8;

			reply.SetID(0);
			reply.AddChar(replycode);
			reply.AddShort(the_world->characters.size());
			reply.AddByte(255);
			UTIL_FOREACH(the_world->characters, character)
			{
				reply.AddBreakString(character->name);
				reply.AddBreakString(character->title);
				reply.AddChar(0); // ?
				reply.AddChar(0); // ?
				reply.AddChar(character->clas);
				reply.AddString("SEX");
				reply.AddByte(255);
			}
			CLIENT_SENDRAW(reply);
		}
		break;

		default:
			return false;
	}

	return true;
}
