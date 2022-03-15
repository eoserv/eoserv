/* npc_data.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "config.hpp"
#include "eodata.hpp"
#include "npc_data.hpp"
#include "world.hpp"

#include "console.hpp"
#include "util.hpp"

#include <algorithm>
#include <array>
#include <cstdio>
#include <string>
#include <vector>

NPC_Data::NPC_Data(World* world, short id)
	: id(id)
	, drops_chance_total(0.0)
	, world(world)
{
	this->LoadShopDrop();
}

const ENF_Data& NPC_Data::ENF() const
{
	return this->world->enf->Get(this->id);
}

void NPC_Data::UnloadShopDrop()
{
	this->drops.clear();
	this->shop_trade.clear();
	this->shop_craft.clear();
	this->skill_learn.clear();

	this->drops_chance_total = 0.0;

	this->citizenship.reset();
}

void NPC_Data::LoadShopDrop()
{
	this->UnloadShopDrop();

	Config::iterator drops = this->world->drops_config.find(util::to_string(this->id));
	if (drops != this->world->drops_config.end())
	{
		std::vector<std::string> parts = util::explode(',', static_cast<std::string>((*drops).second));

		if (parts.size() > 1)
		{
			if (parts.size() % 4 != 0)
			{
				Console::Wrn("skipping invalid drop data for NPC #%i", id);
				return;
			}

			double chance_offset = 0.0;

			this->drops.resize(parts.size() / 4);

			for (std::size_t i = 0; i < parts.size(); i += 4)
			{
				std::unique_ptr<NPC_Drop> drop(new NPC_Drop);

				drop->id = util::to_int(parts[i]);
				drop->min = util::to_int(parts[i+1]);
				drop->max = util::to_int(parts[i+2]);
				drop->chance = util::to_float(parts[i+3]);
				drop->chance_offset = chance_offset;

				chance_offset += drop->chance;

				this->drops[i/4] = std::move(drop);
			}

			if (chance_offset > 100.001)
			{
				this->drops_chance_total = chance_offset;

				if (static_cast<int>(this->world->config["DropRateMode"]) == 3)
					Console::Wrn("Drop rates for NPC #%i add up to %g%%. They have been scaled down proportionally.", this->id, this->drops_chance_total);
			}
			else
			{
				this->drops_chance_total = 100.0;
			}
		}
	}

	short shop_vend_id;

	if (int(this->world->shops_config["Version"]) < 2)
	{
		shop_vend_id = this->id;
	}
	else
	{
		shop_vend_id = this->ENF().vendor_id;
	}

	if (this->ENF().type == ENF::Type::Shop && shop_vend_id > 0)
	{
		this->shop_name = static_cast<std::string>(this->world->shops_config[util::to_string(shop_vend_id) + ".name"]);
		Config::iterator shops = this->world->shops_config.find(util::to_string(shop_vend_id) + ".trade");
		if (shops != this->world->shops_config.end())
		{
			std::vector<std::string> parts = util::explode(',', static_cast<std::string>((*shops).second));

			if (parts.size() > 1)
			{
				if (parts.size() % 3 != 0)
				{
					Console::Wrn("skipping invalid trade shop data for vendor #%i", shop_vend_id);
					return;
				}

				this->shop_trade.resize(parts.size() / 3);

				for (std::size_t i = 0; i < parts.size(); i += 3)
				{
					std::unique_ptr<NPC_Shop_Trade_Item> item(new NPC_Shop_Trade_Item);
					item->id = util::to_int(parts[i]);
					item->buy = util::to_int(parts[i+1]);
					item->sell = util::to_int(parts[i+2]);

					if (item->buy != 0 && item->sell != 0 && item->sell > item->buy)
					{
						Console::Wrn("item #%i (vendor #%i) has a higher sell price than buy price.", item->id, shop_vend_id);
					}

					this->shop_trade[i/3] = std::move(item);
				}
			}
		}

		shops = this->world->shops_config.find(util::to_string(shop_vend_id) + ".craft");
		if (shops != this->world->shops_config.end())
		{
			std::vector<std::string> parts = util::explode(',', static_cast<std::string>((*shops).second));

			if (parts.size() > 1)
			{
				if (parts.size() % 9 != 0)
				{
					Console::Wrn("skipping invalid craft shop data for vendor #%i", shop_vend_id);
					return;
				}

				this->shop_craft.resize(parts.size() / 9);

				for (std::size_t i = 0; i < parts.size(); i += 9)
				{
					std::unique_ptr<NPC_Shop_Craft_Item> item(new NPC_Shop_Craft_Item);
					std::vector<NPC_Shop_Craft_Ingredient> ingredients;
					ingredients.resize(4);

					item->id = util::to_int(parts[i]);

					for (int ii = 0; ii < 4; ++ii)
					{
						NPC_Shop_Craft_Ingredient ingredient;
						ingredient.id = util::to_int(parts[i+1+ii*2]);
						ingredient.amount = util::to_int(parts[i+2+ii*2]);
						ingredients[ii] = ingredient;
					}

					item->ingredients = ingredients;

					this->shop_craft[i/9] = std::move(item);
				}
			}
		}
	}

	short skills_vend_id;

	if (int(this->world->skills_config["Version"]) < 2)
	{
		skills_vend_id = this->id;
	}
	else
	{
		skills_vend_id = this->ENF().vendor_id;
	}

	if (this->ENF().type == ENF::Type::Skills && skills_vend_id > 0)
	{
		this->skill_name = static_cast<std::string>(this->world->skills_config[util::to_string(skills_vend_id) + ".name"]);
		Config::iterator skills = this->world->skills_config.find(util::to_string(skills_vend_id) + ".learn");
		if (skills != this->world->skills_config.end())
		{
			std::vector<std::string> parts = util::explode(',', static_cast<std::string>((*skills).second));

			if (parts.size() > 1)
			{
				if (parts.size() % 14 != 0)
				{
					Console::Err("WARNING: skipping invalid skill learn data for vendor #%i", skills_vend_id);
					return;
				}

				this->skill_learn.resize(parts.size() / 14);

				for (std::size_t i = 0; i < parts.size(); i += 14)
				{
					std::unique_ptr<NPC_Learn_Skill> skill(new NPC_Learn_Skill);

					skill->id = util::to_int(parts[i]);
					skill->cost = util::to_int(parts[i+1]);
					skill->levelreq = util::to_int(parts[i+2]);
					skill->classreq = util::to_int(parts[i+3]);

					skill->skillreq[0] = util::to_int(parts[i+4]);
					skill->skillreq[1] = util::to_int(parts[i+5]);
					skill->skillreq[2] = util::to_int(parts[i+6]);
					skill->skillreq[3] = util::to_int(parts[i+7]);

					skill->strreq = util::to_int(parts[i+8]);
					skill->intreq = util::to_int(parts[i+9]);
					skill->wisreq = util::to_int(parts[i+10]);
					skill->agireq = util::to_int(parts[i+11]);
					skill->conreq = util::to_int(parts[i+12]);
					skill->chareq = util::to_int(parts[i+13]);

					this->skill_learn[i/14] = std::move(skill);
				}
			}
		}
	}

	short home_vend_id;

	if (int(this->world->home_config["Version"]) < 2)
	{
		home_vend_id = this->id;
	}
	else
	{
		home_vend_id = this->ENF().vendor_id;
	}

	if (this->ENF().type == ENF::Type::Inn && home_vend_id > 0)
	{
		restart_loop:
		UTIL_FOREACH(this->world->home_config, hc)
		{
			std::vector<std::string> parts = util::explode('.', hc.first);

			if (parts.size() < 2)
			{
				continue;
			}

			if (!this->citizenship && parts[1] == "innkeeper" && util::to_int(hc.second) == home_vend_id)
			{
				this->citizenship.reset(new NPC_Citizenship);
				this->citizenship->home = parts[0];
				Home* home = this->world->GetHome(this->citizenship->home);

				if (home)
					home->innkeeper_vend = this->ENF().vendor_id;
				else
					Console::Wrn("Vendor #%i's innkeeper set on non-existent home: %s", home_vend_id, this->citizenship->home.c_str());

				// Restart the loop so questions/answers specified before the innkeeper option will load
				goto restart_loop;
			}
			else if (this->citizenship && this->citizenship->home == parts[0] && parts[1].substr(0, parts[1].length() - 1) == "question")
			{
				int index = parts[1][parts[1].length() - 1] - '1';

				if (index < 0 || index >= 3)
				{
					Console::Wrn("Exactly 3 questions must be specified for %s innkeeper vendor #%i", std::string(hc.second).c_str(), home_vend_id);
				}

				this->citizenship->questions[index] = static_cast<std::string>(hc.second);
			}
			else if (this->citizenship && this->citizenship->home == parts[0] && parts[1].substr(0, parts[1].length() - 1) == "answer")
			{
				int index = parts[1][parts[1].length() - 1] - '1';

				if (index < 0 || index >= 3)
				{
					Console::Wrn("Exactly 3 answers must be specified for %s innkeeper vendor #%i", std::string(hc.second).c_str(), home_vend_id);
				}

				this->citizenship->answers[index] = static_cast<std::string>(hc.second);
			}
		}
	}
}

NPC_Data::~NPC_Data()
{

}
