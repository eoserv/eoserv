
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
			UTIL_LIST_FOREACH_ALL(the_world->characters, Character *, character)
			{
				reply.AddBreakString(character->name);
				reply.AddBreakString(character->title);
				reply.AddChar(0); // ?
				if (character->admin >= ADMIN_HGM)
				{
					if (character->party)
					{
						reply.AddChar(ICON_HGM_PARTY);
					}
					else
					{
						reply.AddChar(ICON_HGM);
					}
				}
				else if (character->admin >= ADMIN_GUIDE)
				{
					if (character->party)
					{
						reply.AddChar(ICON_GM_PARTY);
					}
					else
					{
						reply.AddChar(ICON_GM);
					}
				}
				else
				{
					if (character->party)
					{
						reply.AddChar(ICON_PARTY);
					}
					else
					{
						reply.AddChar(ICON_NORMAL);
					}
				}
				reply.AddChar(character->clas);
				reply.AddString(character->PaddedGuildTag());
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
