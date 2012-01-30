
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef QUEST_HPP_INCLUDED
#define QUEST_HPP_INCLUDED

#include <map>
#include <memory>
#include <string>

#include "fwd/character.hpp"
#include "fwd/dialog.hpp"
#include "fwd/world.hpp"

#include "eoplus/context.hpp"

class Quest
{
	private:
		World* world;
		const EOPlus::Quest* quest;
		short id;

		void Load();

	public:
		Quest(short id, World* world);

		const EOPlus::Quest* GetQuest() const { return quest; }
		short ID() const;
		std::string Name() const;

		~Quest();
};

class Quest_Context : public EOPlus::Context
{
	public:
		struct ProgressInfo
		{
			BookIcon icon;
			short progress;
			short target;
		};

	private:
		Character* character;
		const Quest* quest;

		std::string state_name;
		std::string state_desc;
		std::map<short, std::shared_ptr<Dialog>> dialogs;

		std::map<std::string, short> progress;

	protected:
		void BeginState(const std::string& name, const EOPlus::State& state);
		bool DoAction(const EOPlus::Action& action);
		bool CheckRule(const EOPlus::Rule& rule);

	public:
		Quest_Context(Character* character, const Quest* quest);

		const Quest* GetQuest() const;
		const Dialog* GetDialog(short id) const;

		std::string StateName() const;
		std::string Desc() const;
		ProgressInfo Progress() const;

		std::string SerializeProgress() const;
		std::string::const_iterator UnserializeProgress(std::string::const_iterator it, std::string::const_iterator begin);

		bool DialogInput(char link_id);
		bool TalkedNPC(char vendor_id);

		void GotItems(short id, int amount);
		void LostItems(short id, int amount);
		void UsedItem(short id);

		void GotSpell(short id, int level);
		void LostSpell(short id);
		void UsedSpell(short id);

		void KilledNPC(short id);
		void KilledPlayer();

		void EnterMap(short map);
		void LeaveMap(short map);
		void EnterCoord(short map, unsigned char x, unsigned char y);
		void LeaveCoord(short map, unsigned char x, unsigned char y);

		~Quest_Context();
};

#endif // QUEST_HPP_INCLUDED
