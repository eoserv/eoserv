
CLIENT_F_FUNC(Talk)
{
	PacketBuilder reply;

	std::string message;

	switch (action)
	{
		case PACKET_REQUEST: // Guild chat message
		{

		}
		break;

		case PACKET_OPEN: // Party chat messagea
		{

		}
		break;

		case PACKET_REPORT:
		case PACKET_MSG: // Global chat message
		{
			reader.GetByte(); // Ordering byte
			message = reader.GetEndString(); // message

			the_world->Msg(this->player->character, message);
		}
		break;

		case PACKET_TELL: // Private chat message
		{

		}
		break;

		//case PACKET_REPORT: // Public chat message
		{

		}
		break;

		default:
			return false;
	}

	return true;
}
