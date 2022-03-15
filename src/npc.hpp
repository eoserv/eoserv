/* npc.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef NPC_HPP_INCLUDED
#define NPC_HPP_INCLUDED

#include "fwd/npc.hpp"

#include "fwd/character.hpp"
#include "fwd/eodata.hpp"
#include "fwd/map.hpp"
#include "fwd/npc_data.hpp"

#include <array>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * Used by the NPC class to store information about an attacker
 */
struct NPC_Opponent
{
	Character *attacker;
	int damage;
	double last_hit;
};

/**
 * An instance of an NPC created and managed by a Map
 */
class NPC
{
	public:
		bool temporary;
		Direction direction;
		unsigned char x, y;
		NPC *parent;
		bool alive;
		double dead_since;
		double last_act;
		double act_speed;
		int walk_idle_for;
		bool attack;
		int hp;
		int totaldamage;
		std::list<std::unique_ptr<NPC_Opponent>> damagelist;

		Map *map;
		unsigned char index;
		unsigned char spawn_type;
		short spawn_time;
		unsigned char spawn_x, spawn_y;

		int id;

		static void SetSpeedTable(std::array<double, 7> speeds);

		NPC(Map *map, short id, unsigned char x, unsigned char y, unsigned char spawn_type, short spawn_time, unsigned char index, bool temporary = false);

		const NPC_Data& Data() const;
		const ENF_Data& ENF() const;

		void Spawn(NPC *parent = 0);
		void Act();

		bool Walk(Direction);
		void Damage(Character *from, int amount, int spell_id = -1);
		void RemoveFromView(Character *target);
		void Killed(Character *from, int amount, int spell_id = -1);
		void Die(bool show = true);

		void Attack(Character *target);

		void FormulaVars(std::unordered_map<std::string, double> &vars, std::string prefix = "");

		~NPC();
};

#endif // NPC_HPP_INCLUDED
