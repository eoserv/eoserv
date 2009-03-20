
// Prevents exploitation of a buffer overflow in the EO client
void limit_message(std::string &message)
{
	if (message.length() > static_cast<std::size_t>(static_cast<int>(eoserv_config["ChatLength"])))
	{
		message = message.substr(0, static_cast<int>(eoserv_config["ChatLength"])-6) + " [...]";
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
			if (!this->player || !this->player->character) return false;

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
				else if (command.length() >= 2 && command.compare(0,2,"si") == 0 && arguments.size() >= 1 && this->player->character->admin >= static_cast<int>(admin_config["sitem"]))
				{
					int id = static_cast<int>(util::variant(arguments[0]));
					int amount = (arguments.size() >= 2)?static_cast<int>(util::variant(arguments[1])):1;
					if (this->player->character->AddItem(id, amount))
					{
						reply.SetID(PACKET_ITEM, PACKET_GET);
						reply.AddShort(0); // UID
						reply.AddShort(id);
						reply.AddThree(amount);
						reply.AddChar(this->player->character->weight);
						reply.AddChar(this->player->character->maxweight);
						CLIENT_SEND(reply);
					}
				}
				else if (command.length() >= 5 && command.compare(0,5,"warpm") == 0 && arguments.size() >= 1 && this->player->character->admin >= static_cast<int>(admin_config["warpmeto"]))
				{
					std::transform(arguments[0].begin(), arguments[0].end(), arguments[0].begin(), static_cast<int(*)(int)>(std::tolower));
					UTIL_FOREACH(the_world->characters, character)
					{
						if (character->name.compare(arguments[0]) == 0)
						{
							this->player->character->Warp(character->mapid, character->x, character->y, WARP_ANIMATION_ADMIN);
							break;
						}
					}
				}
				else if (command.length() >= 5 && command.compare(0,5,"warpt") == 0 && arguments.size() >= 1 && this->player->character->admin >= static_cast<int>(admin_config["warptome"]))
				{
					std::transform(arguments[0].begin(), arguments[0].end(), arguments[0].begin(), static_cast<int(*)(int)>(std::tolower));
					UTIL_FOREACH(the_world->characters, character)
					{
						if (character->name.compare(arguments[0]) == 0)
						{
							character->Warp(this->player->character->mapid, this->player->character->x, this->player->character->y, WARP_ANIMATION_ADMIN);
							break;
						}
					}
				}
				else if (command.length() >= 1 && command.compare(0,1,"w") == 0 && arguments.size() >= 3 && this->player->character->admin >= static_cast<int>(admin_config["warp"]))
				{
					int map = static_cast<int>(util::variant(arguments[0]));
					int x = static_cast<int>(util::variant(arguments[1]));
					int y = static_cast<int>(util::variant(arguments[2]));

					if (map < 0 || map >= static_cast<int>(the_world->maps.size()))
					{
						break;
					}

					this->player->character->Warp(map, x, y, WARP_ANIMATION_ADMIN);
				}
			}
			else
			{
				this->player->character->map->Msg(this->player->character, message);
			}
		}
		break;

		case PACKET_MOVEADMIN: // Admin chat message
		{
			if (!this->player || !this->player->character) return false;

			if (this->player->character->admin < ADMIN_GUARDIAN) return false;

			message = reader.GetEndString(); // message
			limit_message(message);

			the_world->AdminMsg(this->player->character, message);
		}
		break;

		case PACKET_ANNOUNCE: // Announcement message
		{
			if (!this->player || !this->player->character) return false;

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
