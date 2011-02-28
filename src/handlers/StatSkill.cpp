
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "character.hpp"
#include "player.hpp"

namespace Handlers
{

// Spending a stat point on a skill
void StatSkill_Add(Character *character, PacketReader &reader)
{
	short *stat;
	/*int action = */reader.GetChar();
	int stat_id = reader.GetShort();

	if (character->statpoints <= 0)
	{
		return;
	}

	switch (stat_id)
	{
		case 1: stat = &character->str; break;
		case 2: stat = &character->intl; break;
		case 3: stat = &character->wis; break;
		case 4: stat = &character->agi; break;
		case 5: stat = &character->con; break;
		case 6: stat = &character->cha; break;
		default: return;
	}

	++(*stat);
	--character->statpoints;

	character->CalculateStats();

	PacketBuilder reply(PACKET_STATSKILL, PACKET_PLAYER);
	reply.AddShort(character->statpoints);
	reply.AddShort(character->str);
	reply.AddShort(character->intl);
	reply.AddShort(character->wis);
	reply.AddShort(character->agi);
	reply.AddShort(character->con);
	reply.AddShort(character->cha);
	reply.AddShort(character->maxhp);
	reply.AddShort(character->maxtp);
	reply.AddShort(character->maxsp);
	reply.AddShort(character->maxweight);
	reply.AddShort(character->mindam);
	reply.AddShort(character->maxdam);
	reply.AddShort(character->accuracy);
	reply.AddShort(character->evade);
	reply.AddShort(character->armor);

	character->Send(reply);
}

PACKET_HANDLER_REGISTER(PACKET_STATSKILL)
	Register(PACKET_ADD, StatSkill_Add, Playing)
PACKET_HANDLER_REGISTER_END()

}
