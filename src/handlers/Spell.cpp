/* handlers/Spell.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../config.hpp"
#include "../eodata.hpp"
#include "../map.hpp"
#include "../packet.hpp"
#include "../timer.hpp"
#include "../world.hpp"

#include "../util.hpp"

#include <algorithm>
#include <functional>

namespace Handlers
{

// Begin spell chant
void Spell_Request(Character *character, PacketReader &reader)
{
	unsigned short spell_id = reader.GetShort();
	int timestamp = reader.GetThree();

	character->timestamp = timestamp;

	character->CancelSpell();

	if (character->HasSpell(spell_id))
	{
		const ESF_Data& spell = character->world->esf->Get(spell_id);

		if (spell.id == 0)
			return;

		character->spell_id = spell_id;
		character->spell_event = new TimeEvent(character_cast_spell, character, 0.47 * spell.cast_time + character->SpellCooldownTime(), 1);
		character->world->timer.Register(character->spell_event);

		PacketBuilder builder(PACKET_SPELL, PACKET_REQUEST, 4);
		builder.AddShort(character->PlayerID());
		builder.AddShort(spell_id);

		UTIL_FOREACH(character->map->characters, updatecharacter)
		{
			if (updatecharacter != character && character->InRange(updatecharacter))
			{
				updatecharacter->Send(builder);
			}
		}
	}
}

// Self-targeting spell cast
void Spell_Target_Self(Character *character, PacketReader &reader)
{
	/*unsigned char target_type = */reader.GetChar();
	/*unsigned short spell_id = */reader.GetShort();
	Timestamp timestamp = reader.GetThree();

	if (!character->spell_event && !character->spell_ready)
		return;

	character->spell_target = Character::TargetSelf;
	character->spell_target_id = 0;

	if (character->world->config["EnforceTimestamps"])
	{
		const ESF_Data& spell = character->world->esf->Get(character->spell_id);

		if (timestamp - character->timestamp < (spell.cast_time - 1) * 47 + 35
		 || timestamp - character->timestamp > std::max<int>(spell.cast_time, 1) * 50)
		{
			character->CancelSpell();
			return;
		}
	}

	character->timestamp = timestamp;

	if (character->spell_ready)
		character->SpellAct();
}

// Targeted spell cast
void Spell_Target_Other(Character *character, PacketReader &reader)
{
	unsigned char target_type = reader.GetChar();
	reader.GetChar();
	reader.GetShort();
	/*unsigned short spell_id = */reader.GetShort();
	unsigned short victim_id = reader.GetShort();
	Timestamp timestamp = reader.GetThree();

	if (!character->spell_event && !character->spell_ready)
		return;

	switch (target_type)
	{
		case 1:
			character->spell_target = Character::TargetPlayer;
			character->spell_target_id = victim_id;
			break;

		case 2:
			character->spell_target = Character::TargetNPC;
			character->spell_target_id = victim_id;
			break;

		default:
			character->CancelSpell();
			return;
	}

	if (character->world->config["EnforceTimestamps"])
	{
		const ESF_Data& spell = character->world->esf->Get(character->spell_id);

		if (timestamp - character->timestamp < (spell.cast_time - 1) * 47 + 35
		 || timestamp - character->timestamp > std::max<int>(spell.cast_time, 1) * 50)
		{
			character->CancelSpell();
			return;
		}
	}

	character->timestamp = timestamp;

	if (character->spell_ready)
		character->SpellAct();
}

// Group spell cast
void Spell_Target_Group(Character *character, PacketReader &reader)
{
	/*unsigned short spell_id = */reader.GetShort();
	Timestamp timestamp = reader.GetThree();

	if (!character->spell_event && !character->spell_ready)
		return;

	character->spell_target = Character::TargetGroup;
	character->spell_target_id = 0;

	if (character->world->config["EnforceTimestamps"])
	{
		const ESF_Data& spell = character->world->esf->Get(character->spell_id);

		if (timestamp - character->timestamp < (spell.cast_time - 1) * 47 + 35
		 || timestamp - character->timestamp > std::max<int>(spell.cast_time, 1) * 50)
		{
			character->CancelSpell();
			return;
		}
	}

	character->timestamp = timestamp;

	if (character->spell_ready)
		character->SpellAct();
}

PACKET_HANDLER_REGISTER(PACKET_SPELL)
	Register(PACKET_REQUEST, Spell_Request, Playing);
	Register(PACKET_TARGET_SELF, Spell_Target_Self, Playing);
	Register(PACKET_TARGET_OTHER, Spell_Target_Other, Playing);
	Register(PACKET_TARGET_GROUP, Spell_Target_Group, Playing);
PACKET_HANDLER_REGISTER_END(PACKET_SPELL)

}
