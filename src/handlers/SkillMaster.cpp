
CLIENT_F_FUNC(SkillMaster)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_OPEN: // Talked to a skill master NPC
		// In order to prevent going out of sync with walking, requests will be stored
		// in the same queue.

			break;

		case PACKET_REPLY: // (unsure) Error message from the skill master

			break;

		default:
			return false;
	}

	return true;
}
