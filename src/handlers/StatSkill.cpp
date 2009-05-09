
CLIENT_F_FUNC(StatSkill)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_OPEN: // Talked to a skill master NPC
		{

		}
		break;

		case PACKET_REPLY: // (unsure) Error message from the skill master
		{

		}
		break;

		case PACKET_ADD: // Spending a stat point on a skill
		{
			if (!this->player || !this->player->character) return false;

			short *stat;
			/*int action = */reader.GetChar();
			int stat_id = reader.GetShort();

			if (this->player->character->statpoints <= 0)
			{
				return false;
			}

			switch (stat_id)
			{
				case 1: stat = &this->player->character->str; break;
				case 2: stat = &this->player->character->intl; break;
				case 3: stat = &this->player->character->wis; break;
				case 4: stat = &this->player->character->agi; break;
				case 5: stat = &this->player->character->con; break;
				case 6: stat = &this->player->character->cha; break;
				default: return false;
			}

			++(*stat);
			--this->player->character->statpoints;

			this->player->character->CalculateStats();

			reply.SetID(PACKET_STATSKILL, PACKET_PLAYER);
			reply.AddShort(this->player->character->statpoints);
			reply.AddShort(this->player->character->str);
			reply.AddShort(this->player->character->intl);
			reply.AddShort(this->player->character->wis);
			reply.AddShort(this->player->character->agi);
			reply.AddShort(this->player->character->con);
			reply.AddShort(this->player->character->cha);
			reply.AddShort(this->player->character->maxhp);
			reply.AddShort(this->player->character->maxtp);
			reply.AddShort(this->player->character->maxsp);
			reply.AddShort(this->player->character->maxweight);
			reply.AddShort(this->player->character->mindam);
			reply.AddShort(this->player->character->maxdam);
			reply.AddShort(this->player->character->accuracy);
			reply.AddShort(this->player->character->evade);
			reply.AddShort(this->player->character->armor);

			CLIENT_SEND(reply);
		}
		break;

		default:
			return false;
	}

	return true;
}
