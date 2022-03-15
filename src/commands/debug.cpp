/* commands/debug.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "commands.hpp"

#include "../character.hpp"
#include "../config.hpp"
#include "../eoplus.hpp"
#include "../i18n.hpp"
#include "../map.hpp"
#include "../npc.hpp"
#include "../packet.hpp"
#include "../quest.hpp"
#include "../timer.hpp"
#include "../world.hpp"

#include "../util.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Commands
{

void SpawnItem(const std::vector<std::string>& arguments, Character* from)
{
	int id = util::to_int(arguments[0]);
	int amount = (arguments.size() >= 2) ? util::to_int(arguments[1]) : 1;

	if (from->AddItem(id, amount))
	{
		PacketBuilder reply(PACKET_ITEM, PACKET_GET, 9);
		reply.AddShort(0); // UID
		reply.AddShort(id);
		reply.AddThree(amount);
		reply.AddChar(from->weight);
		reply.AddChar(from->maxweight);
		from->Send(reply);
	}
}

void DropItem(const std::vector<std::string>& arguments, Character* from)
{
	if (from->trading) return;

	int id = util::to_int(arguments[0]);
	int amount = (arguments.size() >= 2) ? util::to_int(arguments[1]) : 1;
	int x = (arguments.size() >= 3) ? util::to_int(arguments[2]) : from->x;
	int y = (arguments.size() >= 4) ? util::to_int(arguments[3]) : from->y;

	if (amount > 0 && from->HasItem(id) >= amount)
	{
		std::shared_ptr<Map_Item> item = from->map->AddItem(id, amount, x, y, from);

		if (item)
		{
			item->owner = from->PlayerID();
			item->unprotecttime = Timer::GetTime() + double(from->world->config["ProtectPlayerDrop"]);
			from->DelItem(id, amount);

			PacketBuilder reply(PACKET_ITEM, PACKET_DROP, 15);
			reply.AddShort(id);
			reply.AddThree(amount);
			reply.AddInt(from->HasItem(id));
			reply.AddShort(item->uid);
			reply.AddChar(x);
			reply.AddChar(y);
			reply.AddChar(from->weight);
			reply.AddChar(from->maxweight);
			from->Send(reply);
		}
	}
}

void SpawnNPC(const std::vector<std::string>& arguments, Character* from)
{
	int id = util::to_int(arguments[0]);
	int amount = (arguments.size() >= 2) ? util::to_int(arguments[1]) : 1;
	unsigned char speed = (arguments.size() >= 3) ? util::to_int(arguments[2]) : int(from->world->config["SpawnNPCSpeed"]);

	short direction = DIRECTION_DOWN;

	if (speed == 7 && arguments.size() >= 4)
	{
		direction = util::to_int(arguments[3]);
	}

	if (id <= 0 || std::size_t(id) > from->world->enf->data.size())
		return;

	for (int i = 0; i < amount; ++i)
	{
		unsigned char index = from->map->GenerateNPCIndex();

		if (index > 250)
			break;

		NPC *npc = new NPC(from->map, id, from->x, from->y, speed, direction, index, true);
		from->map->npcs.push_back(npc);
		npc->Spawn();
	}
}

void DespawnNPC(const std::vector<std::string>& arguments, Character* from)
{
	(void)arguments;

	std::vector<NPC*> despawn_npcs;

	for (NPC* npc : from->map->npcs)
	{
		if (npc->temporary)
		{
			despawn_npcs.push_back(npc);
		}
	}

	for (NPC* npc : despawn_npcs)
	{
		npc->Die();
	}
}

void Learn(const std::vector<std::string>& arguments, Character* from)
{
	short skill_id = util::to_int(arguments[0]);
	short level = -1;

	if (arguments.size() >= 2)
		level = std::max(0, std::min<int>(from->world->config["MaxSkillLevel"], util::to_int(arguments[1])));

	if (from->AddSpell(skill_id))
	{
		PacketBuilder builder(PACKET_STATSKILL, PACKET_TAKE, 6);
		builder.AddShort(skill_id);
		builder.AddInt(from->HasItem(1));
		from->Send(builder);
	}

	if (level >= 0)
	{
		auto it = std::find_if(UTIL_RANGE(from->spells), [&](Character_Spell spell) { return spell.id == skill_id; });

		if (it != from->spells.end())
		{
			it->level = level;

			PacketBuilder builder(PACKET_STATSKILL, PACKET_ACCEPT, 6);
			builder.AddShort(from->skillpoints);
			builder.AddShort(skill_id);
			builder.AddShort(it->level);
			from->Send(builder);
		}
	}
}

void QuestState(const std::vector<std::string>& arguments, Character* from)
{
	World* world = from->SourceWorld();
	short quest_id = util::to_int(arguments[0]);

	std::shared_ptr<Quest_Context> quest = from->GetQuest(quest_id);

	if (!quest)
	{
		auto it = world->quests.find(quest_id);

		if (it != world->quests.end())
		{
			// WARNING: holds a non-tracked reference to shared_ptr
			quest = std::make_shared<Quest_Context>(from, it->second.get());
			from->quests[it->first] = quest;
			quest->SetState("begin", true);
		}
	}

	if (!quest)
	{
		from->ServerMsg(world->i18n.Format("quest_not_found"));
	}
	else
	{
		try
		{
			quest->SetState(arguments[1]);
		}
		catch (EOPlus::Runtime_Error& e)
		{
			from->ServerMsg(world->i18n.Format("quest_state_not_found"));
		}
	}
}

COMMAND_HANDLER_REGISTER(debug)
	RegisterCharacter({"sitem", {"item"}, {"amount"}, 2}, SpawnItem, CMD_FLAG_DUTY_RESTRICT);
	RegisterCharacter({"ditem", {"item"}, {"amount", "x", "y"}, 2}, DropItem, CMD_FLAG_DUTY_RESTRICT);
	RegisterCharacter({"snpc", {"npc"}, {"amount", "speed", "direction"}, 2}, SpawnNPC, CMD_FLAG_DUTY_RESTRICT);
	RegisterCharacter({"dnpc", {}, {}, 2}, DespawnNPC, CMD_FLAG_DUTY_RESTRICT);
	RegisterCharacter({"learn", {"skill"}, {"level"}}, Learn, CMD_FLAG_DUTY_RESTRICT);
	RegisterCharacter({"qstate", {"quest", "state"}}, QuestState, CMD_FLAG_DUTY_RESTRICT);
	RegisterAlias("si", "sitem");
	RegisterAlias("di", "ditem");
	RegisterAlias("sn", "snpc");
	RegisterAlias("dn", "dnpc");
COMMAND_HANDLER_REGISTER_END(debug)

}
