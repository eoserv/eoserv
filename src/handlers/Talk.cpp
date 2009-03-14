
// Prevents exploitation of a buffer overflow in the EO client
void limit_message(std::string &message)
{
	if (message.length() > 128)
	{
		message = message.substr(0, 122) + " [...]";
	}

	// This is the noticeably annoying part of it
	if (message.length() > 96)
	{
		std::transform(message.begin()+64, message.end(), message.begin()+64, static_cast<int(*)(int)>(std::tolower));
	}
}

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

		case PACKET_MSG: // Global chat message
		{
			if (!this->player || !this->player->character) return false;

			message = reader.GetEndString(); // message
			limit_message(message);

			the_world->Msg(this->player->character, message);
		}
		break;

		case PACKET_TELL: // Private chat message
		{

		}
		break;

		case PACKET_REPORT: // Public chat message
		{
			if (!this->player || !this->player->character || !this->player->character->map) return false;

			message = reader.GetEndString(); // message
			limit_message(message);

			if (message.empty())
			{
				return false;
			}

			if (this->player->character->admin && message[0] == '$')
			{
				std::string command;
				std::list<std::string> arguments = util::explode(' ', message);
				command = arguments.front().substr(1);
				arguments.pop_front();
				if (command.length() >= 1 && command.compare(0,1,"k") == 0 && arguments.size() >= 1 && this->player->character->admin >= static_cast<int>(admin_config["kick"]))
				{
					Character *victim = the_world->GetCharacter(arguments.front());
					if (victim)
					{
						the_world->Kick(this->player->character, victim);
					}
				}
			}

			this->player->character->map->Msg(this->player->character, message);
		}
		break;

		case PACKET_MOVEADMIN: // Admin chat message
		{
			if (!this->player || !this->player->character || !this->player->character->map) return false;

			if (this->player->character->admin < ADMIN_GUARDIAN) return false;

			message = reader.GetEndString(); // message
			limit_message(message);

			the_world->AdminMsg(this->player->character, message);
		}
		break;

		case PACKET_ANNOUNCE: // Announcement message
		{
			if (!this->player || !this->player->character || !this->player->character->map) return false;

			if (this->player->character->admin < ADMIN_GUARDIAN) return false;

			message = reader.GetEndString(); // message
			limit_message(message);

			the_world->AnnounceMsg(this->player->character, message);
		}
		break;

		default:
			return false;
	}

	return true;
}
