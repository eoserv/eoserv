
CLIENT_F_FUNC(Warp)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_ACCEPT: // Player accepting a warp request from the server
		{
			if (!this->player || !this->player->character) return false;

			int map = reader.GetShort();

			int anim = this->player->character->warp_anim;

			if (anim && this->player->character->mapid == map)
			{
				this->player->character->warp_anim = 0;
			}
			else
			{
				return true;
			}

			std::list<Character *> updatecharacters;
			std::list<Map_Item> updateitems;
			UTIL_FOREACH(this->player->character->map->characters, character)
			{
				if (this->player->character->InRange(character))
				{
					updatecharacters.push_back(character);
				}
			}
			UTIL_FOREACH(this->player->character->map->items, item)
			{
				if (this->player->character->InRange(item))
				{
					updateitems.push_back(item);
				}
			}
			reply.SetID(PACKET_WARP, PACKET_AGREE);
			reply.AddChar(2); // ?
			reply.AddShort(map);
			reply.AddChar(anim);
			reply.AddChar(updatecharacters.size());
			reply.AddByte(255);
			UTIL_FOREACH(updatecharacters, character)
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
				reply.AddShort(eoserv_items->Get(character->paperdoll[Character::Boots])->dollgraphic);
				reply.AddShort(0); // ??
				reply.AddShort(0); // ??
				reply.AddShort(0); // ??
				reply.AddShort(eoserv_items->Get(character->paperdoll[Character::Armor])->dollgraphic);
				reply.AddShort(0); // ??
				reply.AddShort(eoserv_items->Get(character->paperdoll[Character::Hat])->dollgraphic);
				reply.AddShort(eoserv_items->Get(character->paperdoll[Character::Shield])->dollgraphic);
				reply.AddShort(eoserv_items->Get(character->paperdoll[Character::Weapon])->dollgraphic);
				reply.AddChar(character->sitting);
				reply.AddChar(0); // visible
				reply.AddByte(255);
			}
			reply.AddByte(255);
			UTIL_FOREACH(updateitems, item)
			{
				reply.AddShort(item.uid);
				reply.AddShort(item.id);
				reply.AddChar(item.x);
				reply.AddChar(item.y);
				reply.AddThree(item.amount);
			}
			CLIENT_SEND(reply);

		}
		break;

		case PACKET_TAKE: // Player needs a copy of the map they're being warped to
		{
			if (!this->player || !this->player->character) return false;

			char mapbuf[6] = {0,};
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
