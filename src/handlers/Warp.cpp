
CLIENT_F_FUNC(Warp)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_ACCEPT: // Player accepting a warp request from the server
		{
			if (!this->player || !this->player->character || !this->player->character->map) return false;

			int map = reader.GetShort();

			if (this->player->character->warp_temp && this->player->character->mapid == map)
			{
				this->player->character->warp_temp = false;
			}
			else
			{
				return true;
			}

			reply.SetID(PACKET_WARP, PACKET_AGREE);
			reply.AddChar(2); // ?
			reply.AddShort(map);
			reply.AddChar(0); // ?
			//reply.AddChar(this->player->character->map->characters.size());
			reply.AddChar(0);
			reply.AddByte(255); // ?
			UTIL_FOREACH(this->player->character->map->characters, character)
			{
				reply.AddBreakString(character->name);
				reply.AddShort(character->player->id);
				reply.AddShort(character->mapid);
				reply.AddShort(character->x);
				reply.AddShort(character->y);
				reply.AddChar(character->direction);
				reply.AddChar(6); // ?
				reply.AddString("SEX"); // guild tag
				reply.AddChar(character->level);
				reply.AddChar(character->gender);
				reply.AddChar(character->hairstyle);
				reply.AddChar(character->haircolor);
				reply.AddChar(character->race);
				reply.AddShort(character->maxhp);
				reply.AddShort(character->hp);
				reply.AddShort(character->maxtp);
				reply.AddShort(character->tp);
				// equipment
				reply.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Boots]));
				reply.AddShort(0); // ??
				reply.AddShort(0); // ??
				reply.AddShort(0); // ??
				reply.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Armor]));
				reply.AddShort(0); // ??
				reply.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Hat]));
				reply.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Shield]));
				reply.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Weapon]));
				reply.AddChar(character->sitting);
				reply.AddChar(0); // visible
				reply.AddByte(255);

				reply.AddByte(255); // ?
			}
			reply.AddByte(255); // ?
			CLIENT_SEND(reply);
			//reply.AddChar(this->player->character->map->characters.size()); // ?

			reply.Reset();

			// "Refresh" the screen due to a weird bug where you end up at 0,0 after warping
			reply.SetID(PACKET_REFRESH, PACKET_REPLY);
			reply.AddChar(this->player->character->map->characters.size()); // Number of players
			reply.AddByte(255);
			UTIL_FOREACH(this->player->character->map->characters, character)
			{
				reply.AddBreakString(character->name);
				reply.AddShort(character->player->id);
				reply.AddShort(character->mapid);
				reply.AddShort(character->x);
				reply.AddShort(character->y);
				reply.AddChar(character->direction);
				reply.AddChar(6); // ?
				reply.AddString("SEX"); // guild tag
				reply.AddChar(character->level);
				reply.AddChar(character->gender);
				reply.AddChar(character->hairstyle);
				reply.AddChar(character->haircolor);
				reply.AddChar(character->race);
				reply.AddShort(character->maxhp);
				reply.AddShort(character->hp);
				reply.AddShort(character->maxtp);
				reply.AddShort(character->tp);
				// equipment
				reply.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Boots]));
				reply.AddShort(0); // ??
				reply.AddShort(0); // ??
				reply.AddShort(0); // ??
				reply.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Armor]));
				reply.AddShort(0); // ??
				reply.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Hat]));
				reply.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Shield]));
				reply.AddShort(eoserv_items->GetDollGraphic(character->paperdoll[Character::Weapon]));
				reply.AddChar(character->sitting);
				reply.AddChar(0); // visible
				reply.AddByte(255);
			}
			CLIENT_SEND(reply);

		}
		break;

		case PACKET_TAKE: // Player needs a copy of the map they're being warped to
		{
			if (!this->player || !this->player->character) return false;

			char mapbuf[6] = {0};
			std::sprintf(mapbuf, "%05i", std::abs(this->player->character->mapid));
			std::string filename = "./data/maps/";
			std::string content;
			std::FILE *fh;
			int replycode = 3;

			filename += mapbuf;
			filename += ".emf";

			fh = std::fopen(filename.c_str(), "rb");

			if (!fh)
			{
				std::fprintf(stderr, "Could not load file: %s\n", filename.c_str());
				return false;
			}

			do {
				char buf[4096];
				int len = std::fread(buf, sizeof(char), 4096, fh);
				content.append(buf, len);
			} while (!std::feof(fh));

			std::fclose(fh);

			reply.SetID(0);
			reply.AddChar(replycode);
			reply.AddString(content);
			CLIENT_SENDRAW(reply);
		}
		break;

		default:
			return false;
	}

	return true;
}
