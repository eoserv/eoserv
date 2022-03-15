/* handlers/Quest.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../dialog.hpp"
#include "../eodata.hpp"
#include "../eoplus.hpp"
#include "../npc.hpp"
#include "../packet.hpp"
#include "../quest.hpp"
#include "../world.hpp"

#include "../util.hpp"

#include <cstddef>
#include <deque>
#include <memory>
#include <utility>

namespace Handlers
{

static void open_quest_dialog(Character* character, NPC* npc, int quest_id, int vendor_id)
{
	struct dialog_t
	{
		int quest_id;
		const std::shared_ptr<Quest_Context> quest;
		const Dialog* dialog;
	};

	std::deque<dialog_t> dialogs;
	std::size_t this_dialog = 0;

	UTIL_FOREACH(character->quests, quest)
	{
		if (!quest.second || quest.second->GetQuest()->Disabled())
			continue;

		const Dialog* dialog = quest.second->GetDialog(vendor_id);

		if (dialog)
		{
			dialogs.push_back(dialog_t{quest.first, quest.second, dialog});

			if (quest.first == quest_id)
				this_dialog = dialogs.size() - 1;
		}
	}

	if (!dialogs.empty())
	{
		character->npc = npc;
		character->npc_type = ENF::Quest;

		PacketBuilder reply(PACKET_QUEST, PACKET_DIALOG, 10);
		reply.AddChar(dialogs.size()); // quest count
		reply.AddShort(vendor_id); // ?
		reply.AddShort(dialogs[this_dialog].quest_id);
		reply.AddShort(0); // session
		reply.AddShort(0); // dialog id
		reply.AddByte(255);

		std::size_t reserve = 0;

		UTIL_FOREACH(dialogs, dialog)
		{
			reserve += 3 + dialog.quest->GetQuest()->Name().length();
		}

		reply.ReserveMore(reserve);

		UTIL_FOREACH(dialogs, dialog)
		{
			reply.AddShort(dialog.quest_id);
			reply.AddBreakString(dialog.quest->GetQuest()->Name());
		}

		dialogs[this_dialog].dialog->BuildPacket(reply);

		character->Send(reply);
	}
}

// Talking to a quest NPC
void Quest_Use(Character *character, PacketReader &reader)
{
	short npc_index = reader.GetShort();
	short quest_id = reader.GetShort();

	UTIL_FOREACH(character->map->npcs, npc)
	{
		if (npc->index == npc_index && npc->ENF().type == ENF::Quest && character->InRange(npc))
		{
			short vendor_id = npc->ENF().vendor_id;

			if (vendor_id < 1)
				continue;

			if (quest_id == 0)
				quest_id = vendor_id;

			auto context = character->GetQuest(vendor_id);

			if (!context)
			{
				auto it = character->world->quests.find(vendor_id);

				if (it != character->world->quests.end())
				{
					// WARNING: holds a non-tracked reference to shared_ptr
					Quest* quest = it->second.get();

					if (quest->Disabled())
						continue;

					auto newcontext = std::make_shared<Quest_Context>(character, quest);
					character->quests[it->first] = newcontext;
					newcontext->SetState("begin");
				}
			}
			else if (context->StateName() == "done")
			{
				context->SetState("begin");
			}

			open_quest_dialog(character, npc, quest_id, vendor_id);

			break;
		}
	}
}

// Response to an NPC dialog
void Quest_Accept(Character *character, PacketReader &reader)
{
	/*short session = */reader.GetShort();
	/*short dialog_id = */reader.GetShort();
	short quest_id = reader.GetShort();
	/*short npc_index = */reader.GetShort();
	DialogReply type = DialogReply(reader.GetChar());

	char action = 0;

	if (type == DIALOG_REPLY_LINK)
		action = reader.GetChar();

	if (character->npc_type == ENF::Quest)
	{
		short vendor_id = character->npc->ENF().vendor_id;
		std::shared_ptr<Quest_Context> quest;

		auto it = character->quests.find(quest_id);

		if (it != character->quests.end() && it->second)
		{
			// WARNING: holds a non-tracked reference to shared_ptr
			const Dialog* dialog = it->second->GetDialog(vendor_id);

			if (dialog && (type == DIALOG_REPLY_OK || (type == DIALOG_REPLY_LINK && dialog->CheckLink(action))))
			{
				quest = it->second;
			}
		}

		if (quest && !quest->GetQuest()->Disabled())
		{
			bool result = action ? quest->DialogInput(action) : quest->TalkedNPC(vendor_id);

			// Run dialog for next quest state
			quest = character->GetQuest(quest_id);

			if (result && quest && !quest->GetQuest()->Disabled() && !quest->Finished() && character->npc)
			{
				open_quest_dialog(character, character->npc, quest_id, vendor_id);
			}
		}
	}
}

// Quest history/progress request
void Quest_List(Character *character, PacketReader &reader)
{
	QuestPage page = QuestPage(reader.GetChar());

	PacketBuilder reply(PACKET_QUEST, PACKET_LIST, 4);
	reply.AddChar(page);
	reply.AddShort(character->quests.size());

	std::size_t reserve = 0;

	switch (page)
	{
		case QUEST_PAGE_PROGRESS:
			UTIL_FOREACH(character->quests, q)
			{
				if (q.second && !q.second->Finished() && !q.second->GetQuest()->Disabled() && !q.second->IsHidden())
					reserve += 9 + q.second->GetQuest()->GetQuest()->info.name.length() + q.second->Desc().length();
			}

			reply.ReserveMore(reserve);

			UTIL_FOREACH(character->quests, q)
			{
				if (!q.second || q.second->Finished() || q.second->GetQuest()->Disabled() || q.second->IsHidden())
					continue;

				Quest_Context::ProgressInfo progress = q.second->Progress();

				reply.AddBreakString(q.second->GetQuest()->GetQuest()->info.name);
				reply.AddBreakString(q.second->Desc());
				reply.AddShort(progress.icon);
				reply.AddShort(progress.progress);
				reply.AddShort(progress.target);
				reply.AddByte(255);
			}

			break;

		case QUEST_PAGE_HISTORY:
			UTIL_FOREACH(character->quests, q)
			{
				if (q.second && q.second->Finished() && !q.second->IsHidden())
					reserve += 1 + q.second->GetQuest()->GetQuest()->info.name.length();
			}

			reply.ReserveMore(reserve);

			UTIL_FOREACH(character->quests, q)
			{
				if (q.second && q.second->Finished() && !q.second->IsHidden())
					reply.AddBreakString(q.second->GetQuest()->GetQuest()->info.name);
			}

			break;
	}

	character->Send(reply);
}

PACKET_HANDLER_REGISTER(PACKET_QUEST)
	Register(PACKET_USE, Quest_Use, Playing);
	Register(PACKET_ACCEPT, Quest_Accept, Playing);
	Register(PACKET_LIST, Quest_List, Playing);
PACKET_HANDLER_REGISTER_END(PACKET_QUEST)

}
