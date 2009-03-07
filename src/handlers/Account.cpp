
const int PACKET_ACCOUNT_EXISTS = 1;
const int PACKET_ACCOUNT_NOT_APPROVED = 2;
const int PACKET_ACCOUNT_CREATED = 3;
const int PACKET_ACCOUNT_CHANGE_FAILED = 5;
const int PACKET_ACCOUNT_CHANGED = 6;


CLIENT_F_FUNC(Account)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Check if a character exists
		{
			reader.GetByte(); // Ordering byte
			std::string username = reader.GetEndString();

			reply.SetID(PACKET_ACCOUNT, PACKET_REPLY);
			if (!Player::ValidName(username))
			{
				reply.AddShort(PACKET_ACCOUNT_EXISTS);
				reply.AddChar(0); // ??
				reply.AddString("NO");
			}
			if (Player::Exists(username))
			{
				reply.AddShort(PACKET_ACCOUNT_NOT_APPROVED);
				reply.AddChar(0); // ??
				reply.AddString("NO");
			}
			else
			{
				reply.AddShort(1000);
				reply.AddChar(0); // ??
				reply.AddString("OK");
			}
			CLIENT_SEND(reply);
		}
		break;

		case PACKET_CREATE: // Account creation
		{
			reader.GetByte(); // Ordering byte
			reader.GetShort(); // Account creation "session ID"
			reader.GetByte(); // ?

			std::string username = reader.GetBreakString();
			std::string password = reader.GetBreakString();
			std::string fullname = reader.GetBreakString();
			std::string location = reader.GetBreakString();
			std::string email = reader.GetBreakString();
			std::string computer = reader.GetBreakString();
			std::string hdid = reader.GetBreakString();

			Player::Create(username, password, fullname, location, email, computer, hdid);
			reply.SetID(PACKET_ACCOUNT, PACKET_REPLY);
			reply.AddShort(PACKET_ACCOUNT_CREATED); // Status code

			CLIENT_SEND(reply);
		}
		break;

		case PACKET_AGREE: // Change password
		{
			reader.GetByte(); // Ordering byte

			reply.SetID(PACKET_ACCOUNT, PACKET_REPLY);
			reply.AddShort(PACKET_ACCOUNT_CHANGED); // Reply code
			reply.AddString("OK");
			CLIENT_SEND(reply);
		}
		break;

		default:
			return false;
	}

	return true;
}
