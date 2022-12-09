/* handlers/StatSkill.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../config.hpp"
#include "../eodata.hpp"
#include "../map.hpp"
#include "../npc.hpp"
#include "../npc_data.hpp"
#include "../packet.hpp"
#include "../world.hpp"

#include "../util.hpp"

namespace Handlers
{

// Talking to a skill master NPC
void StatSkill_Open(Character *character, PacketReader &reader)
{
	short id = reader.GetShort();

	UTIL_FOREACH(character->map->npcs, npc)
	{
		if (npc->index == id && npc->Data().skill_learn.size() > 0)
		{
			character->npc = npc;
			character->npc_type = ENF::Skills;

			PacketBuilder reply(PACKET_STATSKILL, PACKET_OPEN, 2 + npc->Data().skill_name.length() + npc->Data().skill_learn.size() * 28);
			reply.AddShort(npc->id);
			reply.AddBreakString(npc->Data().skill_name.c_str());

			UTIL_FOREACH_CREF(npc->Data().skill_learn, skill)
			{
				reply.AddShort(skill->id);
				reply.AddChar(skill->levelreq);
				reply.AddChar(skill->classreq);
				reply.AddInt(skill->cost);
				reply.AddShort(skill->skillreq[0]);
				reply.AddShort(skill->skillreq[1]);
				reply.AddShort(skill->skillreq[2]);
				reply.AddShort(skill->skillreq[3]);
				reply.AddShort(skill->strreq);
				reply.AddShort(skill->wisreq);
				reply.AddShort(skill->intreq);
				reply.AddShort(skill->agireq);
				reply.AddShort(skill->conreq);
				reply.AddShort(skill->chareq);
			}

			character->Send(reply);

			break;
		}
	}
}

// Learning a skill
void StatSkill_Take(Character *character, PacketReader &reader)
{
	/*int shopid = */reader.GetInt();
	short spell_id = reader.GetShort();

	if (character->HasSpell(spell_id))
		return;

	if (character->npc_type == ENF::Skills)
	{
		UTIL_FOREACH_CREF(character->npc->Data().skill_learn, spell)
		{
			if (spell->id == spell_id)
			{
				if (character->level < spell->levelreq || character->HasItem(1) < spell->cost
				 || character->display_str < spell->strreq || character->display_intl < spell->intreq
				 || character->display_wis < spell->wisreq || character->display_agi < spell->agireq
				 || character->display_con < spell->conreq || character->display_cha < spell->chareq)
				{
					// No correct reply for these
					PacketBuilder reply(PACKET_STATSKILL, PACKET_REPLY, 4);
					reply.AddShort(SKILLMASTER_WRONG_CLASS);
					reply.AddShort(character->clas);
					character->Send(reply);
					return;
				}

				if (spell->classreq != 0 && character->clas != spell->classreq)
				{
					PacketBuilder reply(PACKET_STATSKILL, PACKET_REPLY, 4);
					reply.AddShort(SKILLMASTER_WRONG_CLASS);
					reply.AddShort(character->clas);
					character->Send(reply);
					return;
				}

				UTIL_FOREACH(spell->skillreq, req)
				{
					if (req != 0 && !character->HasSpell(req))
					{
						// No correct reply for this
						PacketBuilder reply(PACKET_STATSKILL, PACKET_REPLY, 4);
						reply.AddShort(SKILLMASTER_WRONG_CLASS);
						reply.AddShort(character->clas);
						character->Send(reply);
						return;
					}
				}

				character->DelItem(1, spell->cost);
				character->AddSpell(spell_id);

				PacketBuilder reply(PACKET_STATSKILL, PACKET_TAKE, 6);
				reply.AddShort(spell_id);
				reply.AddInt(character->HasItem(1));
				character->Send(reply);

				break;
			}
		}
	}
}

// Forgeting a skill
void StatSkill_Remove(Character *character, PacketReader &reader)
{
	/*int shopid = */reader.GetInt();
	short spell_id = reader.GetShort();

	if (character->npc_type == ENF::Skills)
	{
		if (character->DelSpell(spell_id))
		{
			PacketBuilder reply(PACKET_STATSKILL, PACKET_REMOVE, 2);
			reply.AddShort(spell_id);
			character->Send(reply);
		}
	}
}

// Spending a stat point on a skill
void StatSkill_Add(Character *character, PacketReader &reader)
{
	int *stat;
	TrainType action = TrainType(reader.GetChar());
	int stat_id = reader.GetShort();

	PacketBuilder reply;

	switch (action)
	{
		case TRAIN_STAT:
			if (character->statpoints <= 0)
				return;

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

			if (*stat >= int(character->world->config["MaxStat"]))
				return;

			++(*stat);
			--character->statpoints;

			character->CalculateStats();

			reply.SetID(PACKET_STATSKILL, PACKET_PLAYER);
			reply.ReserveMore(32);
			reply.AddShort(character->statpoints);
			reply.AddShort(character->display_str);
			reply.AddShort(character->display_intl);
			reply.AddShort(character->display_wis);
			reply.AddShort(character->display_agi);
			reply.AddShort(character->display_con);
			reply.AddShort(character->display_cha);
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

			break;

		case TRAIN_SKILL:
			if (character->skillpoints <= 0 || !character->HasSpell(stat_id)
			 || character->SpellLevel(stat_id) >= int(character->world->config["MaxSkillLevel"]))
			{
				return;
			}

			UTIL_IFOREACH(character->spells, spell)
			{
				if (spell->id == stat_id)
				{
					++spell->level;
					--character->skillpoints;

					reply.SetID(PACKET_STATSKILL, PACKET_ACCEPT);
					reply.ReserveMore(6);
					reply.AddShort(character->skillpoints);
					reply.AddShort(stat_id);
					reply.AddShort(spell->level);
					character->Send(reply);
				}
			}

			break;
	}
}

// Reseting character's skills
void StatSkill_Junk(Character *character, PacketReader &reader)
{
	(void)reader;
	/*int shopid = reader.GetInt();*/

	if (character->npc_type == ENF::Skills)
	{
		character->Reset();

		PacketBuilder builder(PACKET_STATSKILL, PACKET_JUNK, 36);
		builder.AddShort(character->statpoints);
		builder.AddShort(character->skillpoints);
		builder.AddShort(character->hp);
		builder.AddShort(character->maxhp);
		builder.AddShort(character->tp);
		builder.AddShort(character->maxtp);
		builder.AddShort(character->maxsp);
		builder.AddShort(character->display_str);
		builder.AddShort(character->display_intl);
		builder.AddShort(character->display_wis);
		builder.AddShort(character->display_agi);
		builder.AddShort(character->display_con);
		builder.AddShort(character->display_cha);
		builder.AddShort(character->mindam);
		builder.AddShort(character->maxdam);
		builder.AddShort(character->accuracy);
		builder.AddShort(character->evade);
		builder.AddShort(character->armor);

		character->Send(builder);
	}
}

PACKET_HANDLER_REGISTER(PACKET_STATSKILL)
	Register(PACKET_OPEN, StatSkill_Open, Playing);
	Register(PACKET_TAKE, StatSkill_Take, Playing);
	Register(PACKET_REMOVE, StatSkill_Remove, Playing);
	Register(PACKET_ADD, StatSkill_Add, Playing);
	Register(PACKET_JUNK, StatSkill_Junk, Playing);
PACKET_HANDLER_REGISTER_END(PACKET_STATSKILL)

}
