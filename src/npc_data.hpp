/* npc_data.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef NPC_DATA_HPP_INCLUDED
#define NPC_DATA_HPP_INCLUDED

#include "fwd/npc_data.hpp"

#include "fwd/character.hpp"
#include "fwd/eodata.hpp"
#include "fwd/world.hpp"

#include <array>
#include <memory>
#include <string>
#include <vector>

/**
 * Used by the NPC_Data class to store information about an item drop
 */
struct NPC_Drop
{
	unsigned short id;
	int min;
	int max;
	double chance;
	double chance_offset;
};

/**
 * Used by the NPC_Data class to store trade shop data
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
 * Used by the NPC_Data class to store craft shop data
 */
struct NPC_Shop_Craft_Item
{
	unsigned short id;
	std::vector<NPC_Shop_Craft_Ingredient> ingredients;
};

/**
 * Used by the NPC_Data class to store innkeeper citizenship information
 */
struct NPC_Citizenship
{
	std::string home;
	std::array<std::string, 3> questions;
	std::array<std::string, 3> answers;
};

/**
 * Used by the NPC_Data class to store skill master data
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

class NPC_Data
{
	public:
		short id;
		std::vector<std::unique_ptr<NPC_Drop>> drops;
		double drops_chance_total;
		std::string shop_name;
		std::string skill_name;
		std::vector<std::unique_ptr<NPC_Shop_Trade_Item>> shop_trade;
		std::vector<std::unique_ptr<NPC_Shop_Craft_Item>> shop_craft;
		std::vector<std::unique_ptr<NPC_Learn_Skill>> skill_learn;
		std::unique_ptr<NPC_Citizenship> citizenship;

		World* world;

		const ENF_Data& ENF() const;

		void LoadShopDrop();
		void UnloadShopDrop();

		NPC_Data(World* world, short id);
		NPC_Data(const NPC_Data&);

		~NPC_Data();
};

#endif // NPC_DATA_HPP_INCLUDED
