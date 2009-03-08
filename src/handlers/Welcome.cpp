
CLIENT_F_FUNC(Welcome)
{
	PacketBuilder reply;

	unsigned int id;

	switch (action)
	{
		case PACKET_REQUEST: // Selected a character
		{
			if (!this->player) return false;

			reader.GetByte(); // Ordering byte

			reader.GetByte(); // ??
			id = reader.GetThree(); // Character ID

			reply.SetID(PACKET_WELCOME, PACKET_REPLY);

			this->player->character = 0;

			UTIL_FOREACH(this->player->characters, character)
			{
				if (character->id == id)
				{
					this->player->character = character;
					break;
				}
			}

			if (!this->player->character)
			{
#ifdef DEBUG
				std::puts("Selected character not found");
#endif
				return false;
			}

			reply.AddShort(1); // REPLY_WELCOME sub-id
			reply.AddShort(this->player->id);
			reply.AddInt(this->player->character->id); // Character ID
			reply.AddShort(84); // Map ID
			reply.AddChar(55); // ??
			reply.AddChar(165); // ??
			reply.AddChar(217); // ??
			reply.AddChar(55); // ??
			reply.AddChar(30); // ??
			reply.AddShort(7); // ??
			reply.AddChar(56); // ?? **
			reply.AddChar(65); // ?? **
			reply.AddChar(2); // ?? **
			reply.AddChar(200); // ?? **
			reply.AddChar(231); // ?? **
			reply.AddChar(1); // ?? **
			reply.AddChar(103); // ?? **
			reply.AddChar(219); // ?? **
			reply.AddChar(25); // ?? **
			reply.AddChar(122); // ?? **
			reply.AddChar(3); // ?? **
			reply.AddChar(1); // ?? **
			reply.AddChar(47); // ?? **
			reply.AddChar(177); // ?? **
			reply.AddChar(183); // ?? **
			reply.AddChar(45); // ?? **
			reply.AddShort(32); // ?? **
			reply.AddChar(154); // ?? **
			reply.AddChar(77); // ?? **
			reply.AddChar(177); // ?? **
			reply.AddChar(221); // ?? **
			reply.AddShort(8); // ?? **
			reply.AddBreakString(this->player->character->name); // Name
			reply.AddBreakString(this->player->character->title); // Title
			reply.AddBreakString(this->player->character->guild?this->player->character->guild->name:""); // Guild Name
			reply.AddBreakString(this->player->character->guild?this->player->character->guild->name:""); // Guild Rank
			reply.AddChar(1); // ??
			reply.AddString("SEX"); // Guild Tag
			reply.AddChar(0); // ?? **
			reply.AddChar(this->player->character->level); // Level
			reply.AddInt(this->player->character->exp); // Experience
			reply.AddInt(this->player->character->usage); // Usage
			reply.AddShort(this->player->character->hp); // Max HP
			reply.AddShort(this->player->character->maxhp); // HP
			reply.AddShort(this->player->character->tp); // Max TP
			reply.AddShort(this->player->character->maxtp); // TP
			reply.AddShort(this->player->character->maxsp); // Max SP
			reply.AddShort(this->player->character->statpoints); // Stat Points
			reply.AddShort(this->player->character->skillpoints); // Skill Points
			reply.AddShort(this->player->character->karma); // Karma (0-2000)
			reply.AddShort(this->player->character->mindam); // Min Damage
			reply.AddShort(this->player->character->maxdam); // Max Damage
			reply.AddShort(this->player->character->accuracy); // Accuracy
			reply.AddShort(this->player->character->evade); // Evade
			reply.AddShort(this->player->character->armor); // Armor
			if (this->version < 28)
			{
				reply.AddChar(100); // STR
				reply.AddChar(101); // WIS
				reply.AddChar(102); // INT
				reply.AddChar(103); // AGI
				reply.AddChar(104); // CON
				reply.AddChar(105); // CHA
			}
			else
			{
				reply.AddShort(100); // STR
				reply.AddShort(101); // WIS
				reply.AddShort(102); // INT
				reply.AddShort(103); // AGI
				reply.AddShort(104); // CON
				reply.AddShort(105); // CHA
			}
			// paperdoll?
			reply.AddShort(0);
			reply.AddShort(0);
			reply.AddShort(0);
			reply.AddShort(0);
			reply.AddShort(0);
			reply.AddShort(0);
			reply.AddShort(0);
			//reply.AddShort(0);
			//reply.AddShort(0);
			//reply.AddShort(0);
			//reply.AddShort(0);
			//reply.AddShort(0);
			//reply.AddShort(0);
			//reply.AddShort(0);
			//reply.AddShort(0);
			reply.AddChar(0); // ?? **
			reply.AddShort(76); // ?? **
			reply.AddShort(4); // ?? **
			reply.AddChar(24); // ?? **
			reply.AddChar(24); // ?? **
			reply.AddShort(10); // ?? **
			reply.AddShort(10); // ?? **
			reply.AddShort(1); // ?? **
			reply.AddShort(1); // ?? **
			reply.AddChar(0); // ?? **
			reply.AddByte(255);

			CLIENT_SEND(reply);
		}
		break;

		case PACKET_MSG: // Welcome message after you login.
		{
			if (!this->player && !this->player->character) return false;

			reader.GetByte(); // Ordering byte

			reader.GetThree(); // ??
			id = reader.GetInt(); // Character ID

			the_world->Login(this->player->character);

			reply.SetID(PACKET_WELCOME, PACKET_REPLY);

			reply.AddShort(2); // REPLY_WELCOME sub-id
			// MotDs
			reply.AddByte(255);
			reply.AddBreakString("Message 1"); // shown in the scr tab
			reply.AddBreakString("Message 2");
			reply.AddBreakString("Message 3");
			reply.AddBreakString("Message 4");
			reply.AddBreakString("Message 5");
			reply.AddBreakString("Message 6");
			reply.AddBreakString("Message 7");
			reply.AddBreakString("Message 8");
			reply.AddBreakString("Message 9");
			// ??
			reply.AddChar(10); // Weight
			reply.AddChar(100); // Max Weight
			// foreach item {
			reply.AddShort(1); // Item ID
			reply.AddInt(2000000); // Item Amount
			reply.AddShort(201); // Item ID
			reply.AddInt(1); // Item Amount
			// }
			reply.AddByte(255);
			// foreach spell {
			reply.AddShort(1); // Spell ID
			reply.AddShort(100); // Spell Level
			reply.AddShort(2); // Spell ID
			reply.AddShort(100); // Spell Level
			reply.AddShort(18); // Spell ID
			reply.AddShort(100); // Spell Level
			// }
			reply.AddByte(255);
			reply.AddChar(this->player->character->map->characters.size()); // Number of players
			reply.AddByte(255);
			UTIL_FOREACH(this->player->character->map->characters, character)
			{
				reply.AddBreakString(character->name);
				reply.AddShort(character->player->id);
				reply.AddShort(character->mapid); // Map ID
				reply.AddShort(character->x); // Map X
				reply.AddShort(character->y); // Map Y
				reply.AddChar(character->direction); // Direction
				reply.AddChar(6); // ?
				reply.AddString("SEX"); // guild tag
				reply.AddChar(character->level); // Level
				reply.AddChar(character->gender); // sex (0 = female, 1 = male)
				reply.AddChar(character->hairstyle); // hair style
				reply.AddChar(character->haircolor); // hair color
				reply.AddChar(character->race); // race (0 = white, 1 = azn, 2 = nigger, 3 = orc, 4 = skeleton, 5 = panda)
				reply.AddShort(character->maxhp); // Max HP (?)
				reply.AddShort(character->hp); // HP (?)
				reply.AddShort(character->maxtp); // Max TP (?)
				reply.AddShort(character->tp); // TP (?)
				// equipment
				reply.AddShort(0); // ??
				reply.AddShort(0); // ??
				reply.AddShort(0); // ??
				reply.AddShort(0); // shoes
				reply.AddShort(0); // armor
				reply.AddShort(0); // ??
				reply.AddShort(0); // hat
				reply.AddShort(0); // shield
				reply.AddShort(0); // weapon
				reply.AddChar(character->sitting); // standing
				reply.AddChar(0); // visible
				reply.AddByte(255);
			}
			// foreach npc {
			reply.AddChar(1); // NPC ID
			reply.AddShort(1); // Graphic ID
			reply.AddChar(5); // Map X
			reply.AddChar(5); // Map Y
			reply.AddChar(3); // Direction
			// }
			reply.AddByte(255);
			// foreach item {
			reply.AddShort(20); // Display ID
			reply.AddShort(200); // Item ID
			reply.AddChar(7); // Map X
			reply.AddChar(7); // Map Y
			reply.AddInt(INT_MAX); // Amount
			// }
			CLIENT_SEND(reply);
		}
		break;

		default:
			return false;
	}

	return true;
}
