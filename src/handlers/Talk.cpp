
// Prevents exploitation of a buffer overflow in the EO client
void limit_message(std::string &message)
{
	if (message.length() > 128)
	{
		message = message.substr(0, 122) + " [...]";
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
				std::vector<std::string> arguments = util::explode(' ', message);
				command = arguments.front().substr(1);
				arguments.erase(arguments.begin());
				if (command.length() >= 1 && command.compare(0,1,"k") == 0 && arguments.size() >= 1 && this->player->character->admin >= static_cast<int>(admin_config["kick"]))
				{
					Character *victim = the_world->GetCharacter(arguments[0]);
					if (victim)
					{
						the_world->Kick(this->player->character, victim);
					}
				}

				if (command.length() >= 2 && command.compare(0,2,"si") == 0 && arguments.size() >= 1 && this->player->character->admin >= static_cast<int>(admin_config["sitem"]))
				{
					PacketBuilder builder;
					int id = static_cast<int>(util::variant(arguments[0]));
					int amount = (arguments.size() >= 2)?static_cast<int>(util::variant(arguments[1])):1;
					this->player->character->AddItem(id, amount);
					builder.SetID(PACKET_ITEM, PACKET_GET);
					builder.AddShort(236); // ?
					builder.AddShort(id);
					builder.AddThree(amount);
					builder.AddChar(9); // ?
					builder.AddChar(76); // ?
					CLIENT_SEND(builder);
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
