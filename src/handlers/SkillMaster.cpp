
CLIENT_F_FUNC(SkillMaster)
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

		default:
			return false;
	}

	return true;
}
