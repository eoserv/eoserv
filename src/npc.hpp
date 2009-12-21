
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

	SCRIPT_REGISTER_REF_DF(NPC_Opponent)
		SCRIPT_REGISTER_VARIABLE("Character @", attacker);
		SCRIPT_REGISTER_VARIABLE("uint16", damage);
		SCRIPT_REGISTER_VARIABLE("double", last_hit);
	SCRIPT_REGISTER_END()
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

	SCRIPT_REGISTER_REF_DF(NPC_Drop)
		SCRIPT_REGISTER_VARIABLE("uint16", id);
		SCRIPT_REGISTER_VARIABLE("int", min);
		SCRIPT_REGISTER_VARIABLE("int", max);
		SCRIPT_REGISTER_VARIABLE("double", chance);
	SCRIPT_REGISTER_END()
};

/**
 * Used by the NPC class to store trade shop data
 */
struct NPC_Shop_Trade_Item : public Shared
{
	unsigned short id;
	int buy;
	int sell;

	SCRIPT_REGISTER_REF_DF(NPC_Shop_Trade_Item)
		SCRIPT_REGISTER_VARIABLE("uint16", id);
		SCRIPT_REGISTER_VARIABLE("int", buy);
		SCRIPT_REGISTER_VARIABLE("int", sell);
	SCRIPT_REGISTER_END()
};

/**
 * Used by the NPC_Shop_Craft_Item class to store item ingredients
 */
struct NPC_Shop_Craft_Ingredient : public Shared
{
	unsigned short id;
	unsigned char amount;

	SCRIPT_REGISTER_REF_DF(NPC_Shop_Craft_Ingredient)
		SCRIPT_REGISTER_VARIABLE("uint16", id);
		SCRIPT_REGISTER_VARIABLE("uint8", amount);
	SCRIPT_REGISTER_END()
};

/**
 * Used by the NPC class to store craft shop data
 */
struct NPC_Shop_Craft_Item : public Shared
{
	unsigned short id;
	PtrVector<NPC_Shop_Craft_Ingredient> ingredients;

	SCRIPT_REGISTER_REF_DF(NPC_Shop_Craft_Item)
		SCRIPT_REGISTER_VARIABLE("uint16", id);
		SCRIPT_REGISTER_VARIABLE("PtrVector<NPC_Shop_Craft_Ingredient>", ingredients);
	SCRIPT_REGISTER_END()
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

		void Spawn();
		void Act();

		bool Walk(Direction);
		void Damage(Character *from, int amount);

		void Attack(Character *target);

		~NPC();

	static NPC *ScriptFactory(Map *map, short id, unsigned char x, unsigned char y, unsigned char spawn_type, short spawn_time, unsigned char index, bool temporary) { return new NPC(map, id, x, y, spawn_type, spawn_time, index, temporary); }

	SCRIPT_REGISTER_REF(NPC)
		SCRIPT_REGISTER_FACTORY("NPC @f(Map @, int16, uint8, uint8, uint8, int16, uint8, bool)", ScriptFactory);

		SCRIPT_REGISTER_VARIABLE("bool", temporary);
		SCRIPT_REGISTER_VARIABLE("int16", id);
		SCRIPT_REGISTER_VARIABLE("uint8", x);
		SCRIPT_REGISTER_VARIABLE("uint8", y);
		SCRIPT_REGISTER_VARIABLE("Direction", direction);
		SCRIPT_REGISTER_VARIABLE("uint8", spawn_type);
		SCRIPT_REGISTER_VARIABLE("int16", spawn_time);
		SCRIPT_REGISTER_VARIABLE("uint8", spawn_x);
		SCRIPT_REGISTER_VARIABLE("uint8", spawn_y);
		SCRIPT_REGISTER_VARIABLE("NPC @", parent);
		SCRIPT_REGISTER_VARIABLE("PtrVector<NPC_Drop>", drops);
		SCRIPT_REGISTER_VARIABLE("string", shop_name);
		SCRIPT_REGISTER_VARIABLE("PtrVector<NPC_Shop_Trade_Item>", shop_trade);
		SCRIPT_REGISTER_VARIABLE("PtrVector<NPC_Shop_Craft_Item>", shop_craft);
		SCRIPT_REGISTER_VARIABLE("bool", alive);
		SCRIPT_REGISTER_VARIABLE("double", dead_since);
		SCRIPT_REGISTER_VARIABLE("double", last_act);
		SCRIPT_REGISTER_VARIABLE("double", act_speed);
		SCRIPT_REGISTER_VARIABLE("int", walk_idle_for);
		SCRIPT_REGISTER_VARIABLE("bool", attack);
		SCRIPT_REGISTER_VARIABLE("int", hp);
		SCRIPT_REGISTER_VARIABLE("int", totaldamage);
		SCRIPT_REGISTER_VARIABLE("PtrList<NPC_Opponent>", damagelist);
		SCRIPT_REGISTER_VARIABLE("Map @", map);
		SCRIPT_REGISTER_VARIABLE("uint8", index);
		SCRIPT_REGISTER_FUNCTION("ENF_Data @Data()", Data);
		SCRIPT_REGISTER_FUNCTION("void Spawn()", Spawn);
		SCRIPT_REGISTER_FUNCTION("void Act()", Act);
		SCRIPT_REGISTER_FUNCTION("bool Walk(Direction)", Walk);
		SCRIPT_REGISTER_FUNCTION("void Damage(Character @, int)", Walk);
		SCRIPT_REGISTER_FUNCTION("void Attack(Character @)", Attack);
	SCRIPT_REGISTER_END()
};

#endif // NPC_HPP_INCLUDED
