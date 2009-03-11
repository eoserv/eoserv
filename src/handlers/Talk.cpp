
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

			reader.GetByte(); // Ordering byte
			message = reader.GetEndString(); // message

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

			reader.GetByte(); // Ordering byte
			message = reader.GetEndString(); // message

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
				if (command.compare("kick") == 0 && arguments.size() >= 1 && this->player->character->admin >= static_cast<int>(admin_config["kick"]))
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

			reader.GetByte(); // Ordering byte
			message = reader.GetEndString(); // message

			the_world->AdminMsg(this->player->character, message);
		}
		break;

		case PACKET_ANNOUNCE: // Announcement message
		{
			if (!this->player || !this->player->character || !this->player->character->map) return false;

			if (this->player->character->admin < ADMIN_GUARDIAN) return false;

			reader.GetByte(); // Ordering byte
			message = reader.GetEndString(); // message

			the_world->AnnounceMsg(this->player->character, message);
		}
		break;

		default:
			return false;
	}

	return true;
}
