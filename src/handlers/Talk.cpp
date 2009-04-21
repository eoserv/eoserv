
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

			if (this->player->character->mapid == static_cast<int>(eoserv_config["JailMap"]))
			{
				return false;
			}

			message = reader.GetEndString();
			limit_message(message);

			the_world->Msg(this->player->character, message);
		}
		break;

		case PACKET_TELL: // Private chat message
		{
			if (!this->player || !this->player->character) return false;

			std::string name = reader.GetBreakString();
			message = reader.GetEndString();
			limit_message(message);
			Character *to = 0;

			UTIL_LIST_FOREACH_ALL(the_world->characters, Character *, character)
			{
				if (character->name == name)
				{
					to = character;
					break;
				}
			}

			if (to)
			{
				to->Msg(this->player->character, message);
			}
			else
			{
				reply.SetID(PACKET_TALK, PACKET_REPLY);
				reply.AddShort(PACKET_TALK_NOTFOUND);
				reply.AddString(name);
				CLIENT_SEND(reply);
			}
		}
		break;

		case PACKET_REPORT: // Public chat message
		{
			if (!this->player || !this->player->character) return false;

			message = reader.GetEndString();
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
						if (victim->admin < this->player->character->admin)
						{
							the_world->Kick(this->player->character, victim);
						}
					}
				}
				else if (command.length() >= 1 && command.compare(0,1,"b") == 0 && arguments.size() >= 1 && this->player->character->admin >= static_cast<int>(admin_config["ban"]))
				{
					Character *victim = the_world->GetCharacter(arguments[0]);
					double duration = (arguments.size() >= 2)?util::tdparse(arguments[1]):util::tdparse(eoserv_config["DefaultBanLength"]);
					if (victim)
					{
						if (victim->admin < this->player->character->admin)
						{
							the_world->Ban(this->player->character, victim, duration);
						}
					}
				}
				else if (command.length() >= 1 && command.compare(0,1,"j") == 0 && arguments.size() >= 1 && this->player->character->admin >= static_cast<int>(admin_config["jail"]))
				{
					Character *victim = the_world->GetCharacter(arguments[0]);
					if (victim)
					{
						if (victim->admin < this->player->character->admin)
						{
							victim->Warp(static_cast<int>(eoserv_config["JailMap"]), static_cast<int>(eoserv_config["JailX"]), static_cast<int>(eoserv_config["JailY"]), WARP_ANIMATION_ADMIN);
						}
					}
				}
				else if (command.length() >= 2 && command.compare(0,2,"si") == 0 && arguments.size() >= 1 && this->player->character->admin >= static_cast<int>(admin_config["sitem"]))
				{
					int id = util::to_int(arguments[0]);
					int amount = (arguments.size() >= 2)?util::to_int(arguments[1]):1;
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
				else if (command.length() >= 2 && command.compare(0,2,"di") == 0 && arguments.size() >= 1 && this->player->character->admin >= static_cast<int>(admin_config["ditem"]))
				{
					int id = util::to_int(arguments[0]);
					int amount = (arguments.size() >= 2)?util::to_int(arguments[1]):1;
					int x = (arguments.size() >= 3)?util::to_int(arguments[2]):this->player->character->x;
					int y = (arguments.size() >= 4)?util::to_int(arguments[3]):this->player->character->y;
					if (amount > 0 && this->player->character->HasItem(id) >= amount)
					{
						Map_Item *item = this->player->character->map->AddItem(id, amount, x, y, this->player->character);
						if (item)
						{
							item->owner = this->player->id;
							item->unprotecttime = Timer::GetTime() + static_cast<double>(eoserv_config["ProctectPlayerDrop"]);
							this->player->character->DelItem(id, amount);

							reply.SetID(PACKET_ITEM, PACKET_DROP);
							reply.AddShort(id);
							reply.AddThree(amount);
							reply.AddInt(this->player->character->HasItem(id));
							reply.AddShort(item->uid);
							reply.AddChar(x);
							reply.AddChar(y);
							reply.AddChar(this->player->character->weight);
							reply.AddChar(this->player->character->maxweight);
							CLIENT_SEND(reply);
						}
					}
				}
				else if (command.length() >= 5 && command.compare(0,5,"warpm") == 0 && arguments.size() >= 1 && this->player->character->admin >= static_cast<int>(admin_config["warpmeto"]))
				{
					Character *victim = the_world->GetCharacter(arguments[0]);
					if (victim)
					{
						this->player->character->Warp(victim->mapid, victim->x, victim->y, WARP_ANIMATION_ADMIN);
					}
				}
				else if (command.length() >= 5 && command.compare(0,5,"warpt") == 0 && arguments.size() >= 1 && this->player->character->admin >= static_cast<int>(admin_config["warptome"]))
				{
					Character *victim = the_world->GetCharacter(arguments[0]);
					if (victim)
					{
						victim->Warp(this->player->character->mapid, this->player->character->x, this->player->character->y, WARP_ANIMATION_ADMIN);
					}
				}
				else if (command.length() >= 1 && command.compare(0,1,"w") == 0 && arguments.size() >= 3 && this->player->character->admin >= static_cast<int>(admin_config["warp"]))
				{
					int map = util::to_int(arguments[0]);
					int x = util::to_int(arguments[1]);
					int y = util::to_int(arguments[2]);

					if (map < 0 || map >= static_cast<int>(the_world->maps.size()))
					{
						break;
					}

					this->player->character->Warp(map, x, y, WARP_ANIMATION_ADMIN);
				}
				else if (command.length() >= 1 && command.compare(0,1,"r") == 0 && arguments.size() >= 1 && this->player->character->admin >= static_cast<int>(admin_config["rehash"]))
				{
					try
					{
						eoserv_config.Read("config.ini");
						admin_config.Read("admin.ini");
					}
					catch (std::runtime_error)
					{

					}
				}
				else if (command.length() >= 2 && command.compare(0,2,"in") == 0 && arguments.size() >= 1 && this->player->character->admin >= static_cast<int>(admin_config["info"]))
				{
					Character *victim = the_world->GetCharacter(arguments[0]);
					if (victim)
					{
						std::string name = victim->name;
						util::ucfirst(name);
						reply.SetID(PACKET_ADMININTERACT, PACKET_TELL);
						if (victim->admin >= 4)
						{
							reply.AddString("High Game Master ");
						}
						else if (victim->admin >= 3)
						{
							reply.AddString("Game Master ");
						}
						else if (victim->admin >= 2)
						{
							reply.AddString("Guardian ");
						}
						else if (victim->admin >= 1)
						{
							reply.AddString("Light Guide ");
						}
						reply.AddString(name);
						reply.AddString(" ");
						reply.AddBreakString(util::trim(victim->PaddedGuildTag()));
						reply.AddInt(victim->Usage());
						reply.AddByte(255);
						reply.AddByte(255);
						reply.AddInt(victim->exp);
						reply.AddChar(victim->level);
						reply.AddShort(victim->mapid);
						reply.AddShort(victim->x);
						reply.AddShort(victim->y);
						reply.AddShort(victim->hp);
						reply.AddShort(victim->maxhp);
						reply.AddShort(victim->tp);
						reply.AddShort(victim->maxtp);
						reply.AddShort(victim->str);
						reply.AddShort(victim->intl);
						reply.AddShort(victim->wis);
						reply.AddShort(victim->agi);
						reply.AddShort(victim->con);
						reply.AddShort(victim->cha);
						reply.AddShort(victim->maxdam);
						reply.AddShort(victim->mindam);
						reply.AddShort(victim->accuracy);
						reply.AddShort(victim->evade);
						reply.AddShort(victim->armor);
						reply.AddShort(0); // light
						reply.AddShort(0); // dark
						reply.AddShort(0); // fire
						reply.AddShort(0); // water
						reply.AddShort(0); // earth
						reply.AddShort(0); // wind
						reply.AddChar(victim->weight);
						reply.AddChar(victim->maxweight);
						CLIENT_SEND(reply);
					}
				}
				else if (command.length() == 8 && command.compare(0,8,"shutdown") == 0 && this->player->character->admin >= static_cast<int>(admin_config["shutdown"]))
				{
					UTIL_LIST_FOREACH_ALL(the_world->characters, Character *, character)
					{
						character->Save();
						character->player->client->Close();
					}
					std::printf("Server shut down by %s\n", this->player->character->name.c_str());
					std::exit(0);
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
