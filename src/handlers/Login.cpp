
CLIENT_F_FUNC(Login)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Logging in to an account
		{
			reader.GetByte();
			std::string username = reader.GetBreakString();
			std::string password = reader.GetBreakString();

			reply.SetID(PACKET_LOGIN, PACKET_REPLY);

			reply.AddShort(3); // Reply code
			// 1 = Wrong user (shouldn't be used)
			// 2 = Wrong user or password
			// 3 = OK (character list follows)
			// 4 = ?? (banned?)
			// 5 = Already logged in
			reply.AddChar(1); // Number of characters
			reply.AddByte(1); // ??
			reply.AddByte(255); // ??
			reply.AddBreakString(username); // Character name
			reply.AddChar(250); // ??
			reply.AddThree(1337); // character id
			reply.AddChar(100); // level
			reply.AddChar(0); // sex (0 = female, 1 = male)
			reply.AddChar(11); // hair style
			reply.AddChar(8); // hair color
			reply.AddChar(5); // race (0 = white, 1 = azn, 2 = nigger, 3 = orc, 4 = skeleton, 5 = panda)
			reply.AddChar(100); // admin level
			reply.AddShort(0); // shoes
			reply.AddShort(26); // armor
			reply.AddShort(27); // hat
			reply.AddShort(10); // shield
			reply.AddShort(58); // weapon
			reply.AddByte(255); // end of character marker

			CLIENT_SEND(reply);

		} break;

		default:
		{
			return false;
		}
	}

	return true;
}
