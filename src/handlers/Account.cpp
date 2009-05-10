
CLIENT_F_FUNC(Account)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Check if a character exists
		{
			if (this->state != EOClient::Initialized) return false;
			std::string username = reader.GetEndString();

			util::lowercase(username);

			reply.SetID(PACKET_ACCOUNT, PACKET_REPLY);
			if (!Player::ValidName(username))
			{
				reply.AddShort(ACCOUNT_NOT_APPROVED);
				reply.AddString("NO");
			}
			else if (Player::Exists(username))
			{
				reply.AddShort(ACCOUNT_EXISTS);
				reply.AddString("NO");
			}
			else
			{
				reply.AddShort(ACCOUNT_CONTINUE);
				reply.AddString("OK");
			}
			CLIENT_SEND(reply);
		}
		break;

		case PACKET_CREATE: // Account creation
		{
			if (this->state != EOClient::Initialized) return false;

			reader.GetShort(); // Account creation "session ID"
			reader.GetByte(); // ?

			std::string username = reader.GetBreakString();
			std::string password = reader.GetBreakString();
			std::string fullname = reader.GetBreakString();
			std::string location = reader.GetBreakString();
			std::string email = reader.GetBreakString();
			std::string computer = reader.GetBreakString();
			std::string hdid = reader.GetBreakString();

			util::lowercase(username);

			reply.SetID(PACKET_ACCOUNT, PACKET_REPLY);
			if (!Player::ValidName(username))
			{
				reply.AddShort(ACCOUNT_NOT_APPROVED);
				reply.AddString("NO");
			}
			else if (Player::Exists(username))
			{
				reply.AddShort(ACCOUNT_EXISTS);
				reply.AddString("NO");
			}
			else
			{
				util::lowercase(username);

				Player::Create(username, password, fullname, location, email, computer, hdid, static_cast<std::string>(this->GetRemoteAddr()));
				reply.AddShort(ACCOUNT_CREATED);
				reply.AddString("OK");
			}

			CLIENT_SEND(reply);
		}
		break;

		case PACKET_AGREE: // Change password
		{
			if (this->state != EOClient::LoggedIn) return false;

			std::string username = reader.GetBreakString();
			std::string oldpassword = reader.GetBreakString();
			std::string newpassword = reader.GetBreakString();

			if (!Player::ValidName(username))
			{
				reply.SetID(PACKET_ACCOUNT, PACKET_REPLY);
				reply.AddShort(ACCOUNT_NOT_APPROVED);
				reply.AddString("NO");
				CLIENT_SEND(reply);
				return true;
			}
			else if (!Player::Exists(username))
			{
				return true;
			}

			Player *changepass = Player::Login(username, oldpassword);

			if (!changepass)
			{
				reply.SetID(PACKET_ACCOUNT, PACKET_REPLY);
				reply.AddShort(ACCOUNT_CHANGE_FAILED);
				reply.AddString("NO");
				CLIENT_SEND(reply);
				return true;
			}

			changepass->ChangePass(newpassword);

			reply.SetID(PACKET_ACCOUNT, PACKET_REPLY);
			reply.AddShort(ACCOUNT_CHANGED);
			reply.AddString("OK");
			CLIENT_SEND(reply);

			delete changepass;
		}
		break;

		default:
			return false;
	}

	return true;
}
