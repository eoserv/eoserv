
CLIENT_F_FUNC(Welcome)
{
	PacketBuilder reply;

	unsigned int id;

	switch (action)
	{
		case PACKET_REQUEST: // Selected a character
		{
			reader.GetByte(); // Ordering byte

			reader.GetByte(); // ??
			id = reader.GetThree(); // Character ID

			reply.SetID(PACKET_WELCOME, PACKET_REPLY);

			this->player->character = 0;

			UTIL_FOREACH(this->player->characters, character)
			{
				std::printf("\nCHECK %i == %i\n",character->id,id);
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
			reply.AddString("   "); // Guild Tag
			reply.AddChar(0); // ?? **
			reply.AddChar(100); // Level
			reply.AddInt(123); // Experience
			reply.AddInt(0); // Usage
			reply.AddShort(1000); // Max HP
			reply.AddShort(1000); // HP
			reply.AddShort(1001); // Max TP
			reply.AddShort(1001); // TP
			reply.AddShort(1002); // Max SP
			reply.AddShort(123); // Stat Points
			reply.AddShort(321); // Skill Points
			reply.AddShort(2000); // Karma (0-2000)
			reply.AddShort(400); // Min Damage
			reply.AddShort(500); // Max Damage
			reply.AddShort(200); // Accuracy
			reply.AddShort(201); // Evade
			reply.AddShort(202); // Armor
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
			reply.AddShort(0);
			reply.AddShort(0);
			reply.AddShort(0);
			reply.AddShort(0);
			reply.AddShort(0);
			reply.AddShort(0);
			reply.AddShort(0);
			reply.AddShort(0);
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
			reply.AddChar(1); // Number of players
			reply.AddByte(255);
			// foreach player {
			reply.AddBreakString(this->player->character->name);
			reply.AddShort(this->player->character->id);
			reply.AddShort(84); // Map ID
			reply.AddShort(7); // Map X
			reply.AddShort(7); // Map Y
			reply.AddChar(1); // Direction
			reply.AddChar(6); // ?
			reply.AddString("SEX"); // guild tag
			reply.AddChar(100); // Level
			reply.AddChar(0); // sex (0 = female, 1 = male)
			reply.AddChar(11); // hair style
			reply.AddChar(8); // hair color
			reply.AddChar(5); // race (0 = white, 1 = azn, 2 = nigger, 3 = orc, 4 = skeleton, 5 = panda)
			reply.AddShort(100); // Max HP (?)
			reply.AddShort(100); // HP (?)
			reply.AddShort(100); // Max TP (?)
			reply.AddShort(100); // TP (?)
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
			reply.AddChar(0); // standing
			reply.AddChar(0); // visible
			reply.AddByte(255);
			// }
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
			reply.AddThree(10000); // Amount
			// }
			CLIENT_SEND(reply);
		}
		break;

		default:
			return false;
	}

	return true;
}
