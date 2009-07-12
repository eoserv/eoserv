
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef NPC_HPP_INCLUDED
#define NPC_HPP_INCLUDED

#include <vector>

class NPC;

struct NPC_Opponent;
struct NPC_Drop;
struct NPC_Shop_Trade_Item;
struct NPC_Shop_Craft_Ingredient;
struct NPC_Shop_Craft_Item;

#include "map.hpp"

#include "eoconst.hpp"
#include "eodata.hpp"

/**
 * Used by the NPC class to store information about an attacker
 */
struct NPC_Opponent
{
	Character *attacker;
	unsigned short damage;
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
	std::vector<NPC_Shop_Craft_Ingredient> ingredients;
};

/**
 * An instance of an NPC created and managed by a Map
 */
class NPC
{
	public:
		bool temporary;
		short id;
		ENF_Data *data;
		unsigned char x, y;
		Direction direction;
		unsigned char spawn_type;
		short spawn_time;
		unsigned char spawn_x, spawn_y;
		NPC *parent;
		std::vector<NPC_Drop> drops;
		std::string shop_name;
		std::vector<NPC_Shop_Trade_Item> shop_trade;
		std::vector<NPC_Shop_Craft_Item> shop_craft;

		bool alive;
		double dead_since;
		double last_act;
		double act_speed;
		int walk_idle_for;
		bool attack;
		int hp;
		int totaldamage;
		std::list<NPC_Opponent> damagelist;

		Map *map;
		unsigned char index;

		NPC(Map *map, short id, unsigned char x, unsigned char y, unsigned char spawn_type, short spawn_time, unsigned char index, bool temporary = false);

		bool SpawnReady();

		void Spawn();
		void Act();

		bool Walk(Direction);
		void Damage(Character *from, int amount);

		void Attack(Character *target);

		~NPC();
};

#endif // NPC_HPP_INCLUDED
