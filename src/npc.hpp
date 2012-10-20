
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef NPC_HPP_INCLUDED
#define NPC_HPP_INCLUDED

#include "fwd/npc.hpp"

#include <list>
#include <string>
#include <array>
#include <unordered_map>
#include <vector>

#include "fwd/character.hpp"
#include "fwd/eodata.hpp"
#include "fwd/map.hpp"

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
 * Used by the NPC class to store information about an item drop
 */
struct NPC_Drop
{
	unsigned short id;
	int min;
	int max;
	double chance;
};

/**
 * Used by the NPC class to store trade shop data
 */
struct NPC_Shop_Trade_Item
{
	unsigned short id;
	int buy;
	int sell;
};

/**
 * Used by the NPC_Shop_Craft_Item class to store item ingredients
 */
struct NPC_Shop_Craft_Ingredient
{
	unsigned short id;
	unsigned char amount;
};

/**
 * Used by the NPC class to store craft shop data
 */
struct NPC_Shop_Craft_Item
{
	unsigned short id;
	std::vector<NPC_Shop_Craft_Ingredient *> ingredients;
};

/**
 * Used by the NPC class to store innkeeper citizenship information
 */
struct NPC_Citizenship
{
	std::string home;
	std::array<std::string, 3> questions;
	std::array<std::string, 3> answers;
};

/**
 * Used by the NPC class to store skill master data
 */
struct NPC_Learn_Skill
{
	unsigned short id;
	int cost;
    unsigned char levelreq;
    unsigned char classreq;
	std::array<short, 4> skillreq;
	short strreq, intreq, wisreq, agireq, conreq, chareq;
};

/**
 * An instance of an NPC created and managed by a Map
 */
class NPC
{
	public:
		bool temporary;
		short id;
		unsigned char x, y;
		Direction direction;
		unsigned char spawn_type;
		short spawn_time;
		unsigned char spawn_x, spawn_y;
		std::vector<NPC_Drop *> drops;
		std::string shop_name;
		std::string skill_name;
		std::vector<NPC_Shop_Trade_Item *> shop_trade;
		std::vector<NPC_Shop_Craft_Item *> shop_craft;
		std::vector<NPC_Learn_Skill *> skill_learn;
		NPC_Citizenship *citizenship;

		NPC *parent;
		bool alive;
		double dead_since;
		double last_act;
		double act_speed;
		int walk_idle_for;
		bool attack;
		int hp;
		int totaldamage;
		std::list<NPC_Opponent *> damagelist;

		Map *map;
		unsigned char index;

		static void SetSpeedTable(std::array<double, 7> speeds);

		NPC(Map *map, short id, unsigned char x, unsigned char y, unsigned char spawn_type, short spawn_time, unsigned char index, bool temporary = false);
		void LoadShopDrop();

		const ENF_Data& Data() const;

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
