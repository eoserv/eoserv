
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef NPC_HPP_INCLUDED
#define NPC_HPP_INCLUDED

#include "stdafx.h"

#include "character.hpp"

/**
 * Used by the NPC class to store information about an attacker
 */
struct NPC_Opponent : public Shared
{
	Character *attacker;
	unsigned short damage;
	double last_hit;
};

/**
 * Used by the NPC class to store information about an item drop
 */
struct NPC_Drop : public Shared
{
	unsigned short id;
	int min;
	int max;
	double chance;
};

/**
 * Used by the NPC class to store trade shop data
 */
struct NPC_Shop_Trade_Item : public Shared
{
	unsigned short id;
	int buy;
	int sell;
};

/**
 * Used by the NPC_Shop_Craft_Item class to store item ingredients
 */
struct NPC_Shop_Craft_Ingredient : public Shared
{
	unsigned short id;
	unsigned char amount;
};

/**
 * Used by the NPC class to store craft shop data
 */
struct NPC_Shop_Craft_Item : public Shared
{
	unsigned short id;
	PtrVector<NPC_Shop_Craft_Ingredient> ingredients;
};

/**
 * An instance of an NPC created and managed by a Map
 */
class NPC : public Shared
{
	public:
		bool temporary;
		short id;
		unsigned char x, y;
		Direction direction;
		unsigned char spawn_type;
		short spawn_time;
		unsigned char spawn_x, spawn_y;
		NPC *parent;
		PtrVector<NPC_Drop> drops;
		std::string shop_name;
		PtrVector<NPC_Shop_Trade_Item> shop_trade;
		PtrVector<NPC_Shop_Craft_Item> shop_craft;

		bool alive;
		double dead_since;
		double last_act;
		double act_speed;
		int walk_idle_for;
		bool attack;
		int hp;
		int totaldamage;
		PtrList<NPC_Opponent> damagelist;

		Map *map;
		unsigned char index;

		NPC(Map *map, short id, unsigned char x, unsigned char y, unsigned char spawn_type, short spawn_time, unsigned char index, bool temporary = false);

		ENF_Data *Data();

		bool SpawnReady();

		void Spawn();
		void Act();

		bool Walk(Direction);
		void Damage(Character *from, int amount);

		void Attack(Character *target);

		~NPC();

	SCRIPT_REGISTER_REF(NPC)

	SCRIPT_REGISTER_END()
};

#endif // NPC_HPP_INCLUDED
