
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "npc.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <set>
#include <string>
#include <vector>

#include "character.hpp"
#include "config.hpp"
#include "console.hpp"
#include "eoclient.hpp"
#include "eodata.hpp"
#include "map.hpp"
#include "packet.hpp"
#include "party.hpp"
#include "player.hpp"
#include "timer.hpp"
#include "quest.hpp"
#include "world.hpp"

static const double speed_table[8] = {0.9, 0.6, 1.3, 1.9, 3.7, 7.5, 15.0, 0.0};

NPC::NPC(Map *map, short id, unsigned char x, unsigned char y, unsigned char spawn_type, short spawn_time, unsigned char index, bool temporary)
{
	this->map = map;
	this->temporary = temporary;
	this->index = index;
	this->id = id;
	this->spawn_x = this->x = x;
	this->spawn_y = this->y = y;
	this->alive = false;
	this->attack = false;
	this->totaldamage = 0;

	if (spawn_type > 7)
	{
		spawn_type = 7;
	}

	this->spawn_type = spawn_type;
	this->spawn_time = spawn_time;
	this->walk_idle_for = 0;

	if (spawn_type == 7)
	{
		this->direction = static_cast<Direction>(spawn_time & 0x03);
		this->spawn_time = 0;
	}
	else
	{
		this->direction = DIRECTION_DOWN;
	}

	this->parent = 0;

	this->citizenship = 0;

	this->LoadShopDrop();
}

void NPC::LoadShopDrop()
{
	this->drops.clear();
	this->shop_trade.clear();
	this->shop_craft.clear();
	this->skill_learn.clear();

	Config::iterator drops = map->world->drops_config.find(util::to_string(this->id));
	if (drops != map->world->drops_config.end())
	{
		std::vector<std::string> parts = util::explode(',', static_cast<std::string>((*drops).second));

		if (parts.size() > 1)
		{
			if (parts.size() % 4 != 0)
			{
				Console::Wrn("skipping invalid drop data for NPC #%i", id);
				return;
			}

			this->drops.resize(parts.size() / 4);

			for (std::size_t i = 0; i < parts.size(); i += 4)
			{
				NPC_Drop *drop(new NPC_Drop);

				drop->id = util::to_int(parts[i]);
				drop->min = util::to_int(parts[i+1]);
				drop->max = util::to_int(parts[i+2]);
				drop->chance = util::to_float(parts[i+3]);

				this->drops[i/4] = drop;
			}
		}
	}

	this->shop_name = static_cast<std::string>(map->world->shops_config[util::to_string(this->id) + ".name"]);
	Config::iterator shops = map->world->shops_config.find(util::to_string(this->id) + ".trade");
	if (shops != map->world->shops_config.end())
	{
		std::vector<std::string> parts = util::explode(',', static_cast<std::string>((*shops).second));

		if (parts.size() > 1)
		{
			if (parts.size() % 3 != 0)
			{
				Console::Wrn("skipping invalid trade shop data for NPC #%i", id);
				return;
			}

			this->shop_trade.resize(parts.size() / 3);

			for (std::size_t i = 0; i < parts.size(); i += 3)
			{
				NPC_Shop_Trade_Item *item(new NPC_Shop_Trade_Item);
				item->id = util::to_int(parts[i]);
				item->buy = util::to_int(parts[i+1]);
				item->sell = util::to_int(parts[i+2]);

				if (item->buy != 0 && item->sell != 0 && item->sell > item->buy)
				{
					Console::Wrn("item #%i (NPC #%i) has a higher sell price than buy price.", item->id, id);
				}

				this->shop_trade[i/3] = item;
			}
		}
	}

	shops = this->map->world->shops_config.find(util::to_string(this->id) + ".craft");
	if (shops != this->map->world->shops_config.end())
	{
		std::vector<std::string> parts = util::explode(',', static_cast<std::string>((*shops).second));

		if (parts.size() > 1)
		{
			if (parts.size() % 9 != 0)
			{
				Console::Wrn("skipping invalid craft shop data for NPC #%i", id);
				return;
			}

			this->shop_craft.resize(parts.size() / 9);

			for (std::size_t i = 0; i < parts.size(); i += 9)
			{
				NPC_Shop_Craft_Item *item = new NPC_Shop_Craft_Item;
				std::vector<NPC_Shop_Craft_Ingredient *> ingredients;
				ingredients.resize(4);

				item->id = util::to_int(parts[i]);

				for (int ii = 0; ii < 4; ++ii)
				{
					NPC_Shop_Craft_Ingredient *ingredient = new NPC_Shop_Craft_Ingredient;
					ingredient->id = util::to_int(parts[i+1+ii*2]);
					ingredient->amount = util::to_int(parts[i+2+ii*2]);
					ingredients[ii] = ingredient;
				}

				item->ingredients = ingredients;

				this->shop_craft[i/9] = item;
			}
		}
	}

    this->skill_name = static_cast<std::string>(map->world->skills_config[util::to_string(this->id) + ".name"]);
    Config::iterator skills = this->map->world->skills_config.find(util::to_string(this->id) + ".learn");
	if (skills != this->map->world->skills_config.end())
	{
		std::vector<std::string> parts = util::explode(',', static_cast<std::string>((*skills).second));

		if (parts.size() > 1)
		{
			if (parts.size() % 14 != 0)
			{
				Console::Err("WARNING: skipping invalid skill learn data for NPC #%i", id);
				return;
			}

			this->skill_learn.resize(parts.size() / 14);

			for (std::size_t i = 0; i < parts.size(); i += 14)
			{
				NPC_Learn_Skill *skill = new NPC_Learn_Skill;

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

				this->skill_learn[i/14] = skill;
			}
		}
	}

	this->citizenship = 0;

	restart_loop:
	UTIL_FOREACH(this->map->world->home_config, hc)
	{
		std::vector<std::string> parts = util::explode('.', hc.first);

		if (parts.size() < 2)
		{
			continue;
		}

		if (!this->citizenship && parts[1] == "innkeeper" && util::to_int(hc.second) == this->id)
		{
			this->citizenship = new NPC_Citizenship;
			this->citizenship->home = parts[0];
			goto restart_loop;
		}
		else if (this->citizenship && this->citizenship->home == parts[0] && parts[1].substr(0, parts[1].length() - 1) == "question")
		{
			int index = parts[1][parts[1].length() - 1] - '1';

			if (index < 0 || index >= 3)
			{
				Console::Wrn("Exactly 3 questions must be specified");
			}

			this->citizenship->questions[index] = static_cast<std::string>(hc.second);
		}
		else if (this->citizenship && this->citizenship->home == parts[0] && parts[1].substr(0, parts[1].length() - 1) == "answer")
		{
			int index = parts[1][parts[1].length() - 1] - '1';

			if (index < 0 || index >= 3)
			{
				Console::Wrn("Exactly 3 answers must be specified");
			}

			this->citizenship->answers[index] = static_cast<std::string>(hc.second);
		}
	}
}

ENF_Data *NPC::Data()
{
	return this->map->world->enf->Get(id);
}

void NPC::Spawn(NPC *parent)
{
	if (this->Data()->boss && !parent)
	{
		UTIL_FOREACH(this->map->npcs, npc)
		{
			if (npc->Data()->child)
			{
				npc->Spawn(this);
			}
		}
	}

	if (parent)
	{
		this->parent = parent;
	}

	if (this->spawn_type < 7)
	{
		bool found = false;
		for (int i = 0; i < 200; ++i)
		{
			if (this->temporary && i == 0)
			{
				this->x = this->spawn_x;
				this->y = this->spawn_y;
			}
			else
			{
				this->x = util::rand(this->spawn_x-2, this->spawn_x+2);
				this->y = util::rand(this->spawn_y-2, this->spawn_y+2);
			}

			if (this->map->Walkable(this->x, this->y, true) && (i > 100 || !this->map->Occupied(this->x, this->y, Map::NPCOnly)))
			{
				this->direction = static_cast<Direction>(util::rand(0,3));
				found = true;
				break;
			}
		}

		if (!found)
		{
			Console::Wrn("An NPC on map %i at %i,%i is being placed by linear scan of spawn area (%s)", this->map->id, this->spawn_x, this->spawn_y, this->map->world->enf->Get(this->id)->name.c_str());
			for (this->x = this->spawn_x-2; this->x <= spawn_x+2; ++this->x)
			{
				for (this->y = this->spawn_y-2; this->y <= this->spawn_y+2; ++this->y)
				{
					if (this->map->Walkable(this->x, this->y, true))
					{
						Console::Wrn("Placed at valid location: %i,%i", this->x, this->y);
						found = true;
						goto end_linear_scan;
					}
				}
			}
		}
		end_linear_scan:

		if (!found)
		{
			Console::Err("NPC couldn't spawn anywhere valid!");
		}
	}

	this->alive = true;
	this->hp = this->Data()->hp;
	this->last_act = Timer::GetTime();
	this->act_speed = speed_table[this->spawn_type];

	PacketBuilder builder(PACKET_APPEAR, PACKET_REPLY, 8);
	builder.AddChar(0);
	builder.AddByte(255);
	builder.AddChar(this->index);
	builder.AddShort(this->id);
	builder.AddChar(this->x);
	builder.AddChar(this->y);
	builder.AddChar(this->direction);

	UTIL_FOREACH(this->map->characters, character)
	{
		if (character->InRange(this))
		{
			character->Send(builder);
		}
	}
}

void NPC::Act()
{
	// Needed for the server startup spawn to work properly
	if (this->Data()->child && !this->parent)
	{
		UTIL_FOREACH(this->map->npcs, npc)
		{
			if (npc->Data()->boss)
			{
				this->parent = npc;
				break;
			}
		}
	}

	this->last_act += double(util::rand(int(this->act_speed * 750.0), int(this->act_speed * 1250.0))) / 1000.0;

	if (this->spawn_type == 7)
	{
		return;
	}

	Character *attacker = 0;
	unsigned char attacker_distance = static_cast<int>(this->map->world->config["NPCChaseDistance"]);
	unsigned short attacker_damage = 0;

	UTIL_FOREACH(this->damagelist, opponent)
	{
		if (opponent->attacker->map != this->map || opponent->attacker->nowhere || opponent->last_hit < Timer::GetTime() - static_cast<double>(this->map->world->config["NPCBoredTimer"]))
		{
			continue;
		}

		int distance = util::path_length(opponent->attacker->x, opponent->attacker->y, this->x, this->y);

		if ((distance < attacker_distance) || (distance == attacker_distance && opponent->damage > attacker_damage))
		{
			attacker = opponent->attacker;
			attacker_damage = opponent->damage;
			attacker_distance = distance;
		}
	}

	if (this->parent)
	{
		UTIL_FOREACH(this->parent->damagelist, opponent)
		{
			if (opponent->attacker->map != this->map || opponent->attacker->nowhere || opponent->last_hit < Timer::GetTime() - static_cast<double>(this->map->world->config["NPCBoredTimer"]))
			{
				continue;
			}
			int distance = util::path_length(opponent->attacker->x, opponent->attacker->y, this->x, this->y);

			if ((distance < attacker_distance) || (distance == attacker_distance && opponent->damage > attacker_damage))
			{
				attacker = opponent->attacker;
				attacker_damage = opponent->damage;
				attacker_distance = distance;
			}
		}
	}

	if (this->Data()->type == ENF::Aggressive && !attacker)
	{
		Character *closest = 0;
		unsigned char closest_distance = static_cast<int>(this->map->world->config["NPCChaseDistance"]);

		UTIL_FOREACH(this->map->characters, character)
		{
			int distance = util::path_length(character->x, character->y, this->x, this->y);

			if (distance < closest_distance)
			{
				closest = character;
				closest_distance = distance;
			}
		}

		if (closest)
		{
			attacker = closest;
		}
	}

	if (attacker)
	{
		int xdiff = this->x - attacker->x;
		int ydiff = this->y - attacker->y;
		int absxdiff = std::abs(xdiff);
		int absydiff = std::abs(ydiff);

		if ((absxdiff == 1 && absydiff == 0) || (absxdiff == 0 && absydiff == 1) || (absxdiff == 0 && absydiff == 0))
		{
			this->Attack(attacker);
			return;
		}
		else if (absxdiff > absydiff)
		{
			if (xdiff < 0)
			{
				this->direction = DIRECTION_RIGHT;
			}
			else
			{
				this->direction = DIRECTION_LEFT;
			}
		}
		else
		{
			if (ydiff < 0)
			{
				this->direction = DIRECTION_DOWN;
			}
			else
			{
				this->direction = DIRECTION_UP;
			}
		}

		if (!this->Walk(this->direction))
		{
			this->Walk(static_cast<Direction>(util::rand(0,3)));
		}
	}
	else
	{
		// Random walking

		int act;
		if (this->walk_idle_for == 0)
		{
			act = util::rand(1,10);
		}
		else
		{
			--this->walk_idle_for;
			act = 11;
		}

		if (act >= 1 && act <= 6) // 60% chance walk foward
		{
			this->Walk(this->direction);
		}

		if (act >= 7 && act <= 9) // 30% change direction
		{
			this->Walk(static_cast<Direction>(util::rand(0,3)));
		}

		if (act == 10) // 10% take a break
		{
			this->walk_idle_for = util::rand(1,4);
		}
	}
}

bool NPC::Walk(Direction direction)
{
	return this->map->Walk(this, direction);
}

void NPC::Damage(Character *from, int amount, int spell_id)
{
	int limitamount = std::min(this->hp, amount);

	if (this->map->world->config["LimitDamage"])
	{
		amount = limitamount;
	}

	this->hp -= amount;
	this->totaldamage += limitamount;

	NPC_Opponent *opponent(new NPC_Opponent);
	bool found = false;

	UTIL_FOREACH(this->damagelist, checkopp)
	{
		if (checkopp->attacker == from)
		{
			found = true;
			checkopp->damage += limitamount;
			checkopp->last_hit = Timer::GetTime();
		}
	}

	if (!found)
	{
		opponent->attacker = from;
		opponent->damage = limitamount;
		opponent->last_hit = Timer::GetTime();
		this->damagelist.push_back(opponent);
		opponent->attacker->unregister_npc.push_back(this);
	}

	if (this->hp > 0)
	{
		PacketBuilder builder(spell_id == -1 ? PACKET_NPC : PACKET_CAST, PACKET_REPLY, 14);

		if (spell_id != -1)
			builder.AddShort(spell_id);

		builder.AddShort(from->player->id);
		builder.AddChar(from->direction);
		builder.AddShort(this->index);
		builder.AddThree(amount);
		builder.AddShort(int(double(this->hp) / double(this->Data()->hp) * 100.0));

		if (spell_id != -1)
			builder.AddShort(from->tp);
		else
			builder.AddChar(1); // ?

		UTIL_FOREACH(this->map->characters, character)
		{
			if (character->InRange(this))
			{
				character->Send(builder);
			}
		}
	}
	else
	{
		this->Killed(from, amount, spell_id);
	}
}

void NPC::RemoveFromView(Character *target)
{
	PacketBuilder builder(PACKET_NPC, PACKET_PLAYER, 7);
	builder.AddChar(this->index);
	if (target->x > 200 && target->y > 200)
	{
		builder.AddChar(0); // x
		builder.AddChar(0); // y
	}
	else
	{
		builder.AddChar(252); // x
		builder.AddChar(252); // y
	}
	builder.AddChar(0); // direction
	builder.AddByte(255);
	builder.AddByte(255);
	builder.AddByte(255);

	PacketBuilder builder2(PACKET_NPC, PACKET_SPEC, 5);
	builder2.AddShort(0); // killer pid
	builder2.AddChar(0); // killer direction
	builder2.AddShort(this->index);
/*
	builder2.AddShort(0); // dropped item uid
	builder2.AddShort(0); // dropped item id
	builder2.AddChar(this->x);
	builder2.AddChar(this->y);
	builder2.AddInt(0); // dropped item amount
	builder2.AddThree(0); // damage
*/

	target->Send(builder);
	target->Send(builder2);
}

void NPC::Killed(Character *from, int amount, int spell_id)
{
	double droprate = this->map->world->config["DropRate"];
	double exprate = this->map->world->config["ExpRate"];
	int sharemode = this->map->world->config["ShareMode"];
	int partysharemode = this->map->world->config["PartyShareMode"];
	std::set<Party *> parties;

	int most_damage_counter = 0;
	Character *most_damage = 0;
	this->alive = false;

	this->dead_since = int(Timer::GetTime());

	std::vector<NPC_Drop *> drops;
	NPC_Drop *drop = 0;
	UTIL_FOREACH(this->drops, checkdrop)
	{
		if ((double(util::rand(0,10000)) / 100.0) < checkdrop->chance * droprate)
		{
			drops.push_back(checkdrop);
		}
	}

	if (drops.size() > 0)
	{
		drop = drops[util::rand(0, drops.size()-1)];
	}

	if (sharemode == 1)
	{
		UTIL_FOREACH(this->damagelist, opponent)
		{
			if (opponent->damage > most_damage_counter)
			{
				most_damage_counter = opponent->damage;
				most_damage = opponent->attacker;
			}
		}
	}

	int dropuid = 0;
	int dropid = 0;
	int dropamount = 0;
	if (drop)
	{
		dropuid = this->map->GenerateItemID();
		dropid = drop->id;
		dropamount = util::rand(drop->min, drop->max);
		Map_Item *newitem(new Map_Item(dropuid, dropid, dropamount, this->x, this->y, from->player->id, Timer::GetTime() + static_cast<int>(this->map->world->config["ProtectNPCDrop"])));
		this->map->items.push_back(newitem);

		// Selects a random number between 0 and maxhp, and decides the winner based on that
		switch (sharemode)
		{
			case 0:
				this->map->items.back()->owner = from->player->id;
				break;

			case 1:
				this->map->items.back()->owner = most_damage->player->id;
				break;

			case 2:
			{
				int rewarded_hp = util::rand(0, this->Data()->hp);
				int count_hp = 0;
				UTIL_FOREACH(this->damagelist, opponent)
				{
					if (opponent->attacker->InRange(this))
					{
						if (rewarded_hp >= count_hp && rewarded_hp < opponent->damage)
						{
							this->map->items.back()->owner = opponent->attacker->player->id;
							break;
						}

						count_hp += opponent->damage;
					}
				}
			}
				break;

			case 3:
			{
				int rand = util::rand(0, this->damagelist.size());
				int i = 0;
				UTIL_FOREACH(this->damagelist, opponent)
				{
					if (opponent->attacker->InRange(this))
					{
						if (rand == i++)
						{
							this->map->items.back()->owner = opponent->attacker->player->id;
							break;
						}
					}
				}
			}
				break;
		}
	}

	UTIL_FOREACH(this->map->characters, character)
	{
		std::list<NPC_Opponent *>::iterator findopp = this->damagelist.begin();
		for (; findopp != this->damagelist.end() && (*findopp)->attacker != character; ++findopp); // no loop body

		if (findopp != this->damagelist.end() || character->InRange(this))
		{
			bool level_up = false;

			PacketBuilder builder(spell_id == -1 ? PACKET_NPC : PACKET_CAST, PACKET_SPEC, 26);

			if (this->Data()->exp != 0)
			{
				if (findopp != this->damagelist.end())
				{
					int reward;
					switch (sharemode)
					{
						case 0:
							if (character == from)
							{
								reward = int(std::ceil(double(this->Data()->exp) * exprate));

								if (reward > 0)
								{
									if (partysharemode)
									{
										if (character->party)
										{
											character->party->ShareEXP(reward, partysharemode, this->map);
										}
										else
										{
											character->exp += reward;
										}
									}
									else
									{
										character->exp += reward;
									}
								}
							}
							break;

						case 1:
							if (character == most_damage)
							{
								reward = int(std::ceil(double(this->Data()->exp) * exprate));

								if (reward > 0)
								{
									if (partysharemode)
									{
										if (character->party)
										{
											character->party->ShareEXP(reward, partysharemode, this->map);
										}
										else
										{
											character->exp += reward;
										}
									}
									else
									{
										character->exp += reward;
									}
								}
							}
							break;

						case 2:
							reward = int(std::ceil(double(this->Data()->exp) * exprate * (double((*findopp)->damage) / double(this->totaldamage))));

							if (reward > 0)
							{
								if (partysharemode)
								{
									if (character->party)
									{
										character->party->temp_expsum += reward;
										parties.insert(character->party);
									}
									else
									{
										character->exp += reward;
									}
								}
								else
								{
									character->exp += reward;
								}
							}
							break;

						case 3:
							reward = int(std::ceil(double(this->Data()->exp) * exprate * (double(this->damagelist.size()) / 1.0)));

							if (reward > 0)
							{
								if (partysharemode)
								{
									if (character->party)
									{
										character->party->temp_expsum += reward;
									}
									else
									{
										character->exp += reward;
									}
								}
								else
								{
									character->exp += reward;
								}
							}
							break;
					}

					character->exp = std::min(character->exp, static_cast<int>(this->map->world->config["MaxExp"]));

					while (character->level < static_cast<int>(this->map->world->config["MaxLevel"]) && character->exp >= this->map->world->exp_table[character->level+1])
					{
						level_up = true;
						++character->level;
						character->statpoints += static_cast<int>(this->map->world->config["StatPerLevel"]);
						character->skillpoints += static_cast<int>(this->map->world->config["SkillPerLevel"]);
						character->CalculateStats();
					}

					if (level_up)
					{
						builder.SetID(spell_id == -1 ? PACKET_NPC : PACKET_CAST, PACKET_ACCEPT);
						builder.ReserveMore(33);
					}
				}
			}

			if (spell_id != -1)
				builder.AddShort(spell_id);

			builder.AddShort(from->player->id);
			builder.AddChar(from->direction);
			builder.AddShort(this->index);
			builder.AddShort(dropuid);
			builder.AddShort(dropid);
			builder.AddChar(this->x);
			builder.AddChar(this->y);
			builder.AddInt(dropamount);
			builder.AddThree(amount);

			if (spell_id != -1)
				builder.AddShort(from->tp);

			if ((sharemode == 0 && character == from) || (sharemode != 0 && findopp != this->damagelist.end()))
			{
				builder.AddInt(character->exp);
			}

			if (level_up)
			{
				builder.AddChar(character->level);
				builder.AddShort(character->statpoints);
				builder.AddShort(character->skillpoints);
				builder.AddShort(character->maxhp);
				builder.AddShort(character->maxtp);
				builder.AddShort(character->maxsp);
			}

			character->Send(builder);
		}
	}

	UTIL_FOREACH(parties, party)
	{
		party->ShareEXP(party->temp_expsum, partysharemode, this->map);
		party->temp_expsum = 0;
	}

	UTIL_FOREACH(this->damagelist, opponent)
	{
		opponent->attacker->unregister_npc.erase(
			std::remove(UTIL_RANGE(opponent->attacker->unregister_npc), this),
			opponent->attacker->unregister_npc.end()
		);
	}

	this->damagelist.clear();
	this->totaldamage = 0;

	short childid = -1;

	if (this->Data()->boss)
	{
		UTIL_FOREACH(this->map->npcs, npc)
		{
			if (npc->Data()->child && !npc->Data()->boss)
			{
				if (childid == -1 || childid == npc->Data()->id)
				{
					npc->Die(false);
					childid = npc->Data()->id;
				}
				else
				{
					npc->Die(true);
				}
			}
		}
	}

	if (childid != -1)
	{
		PacketBuilder builder(PACKET_NPC, PACKET_JUNK, 2);
		builder.AddShort(childid);

		UTIL_FOREACH(this->map->characters, character)
		{
			character->Send(builder);
		}
	}

	if (this->temporary)
	{
		this->map->npcs.erase(
			std::remove(this->map->npcs.begin(), this->map->npcs.end(), this),
			this->map->npcs.end()
		);
	}

	UTIL_FOREACH(from->quests, q) { q.second->KilledNPC(this->Data()->id); }
}

void NPC::Die(bool show)
{
	this->alive = false;
	this->dead_since = int(Timer::GetTime());

	UTIL_FOREACH(this->damagelist, opponent)
	{
		opponent->attacker->unregister_npc.erase(
			std::remove(UTIL_RANGE(opponent->attacker->unregister_npc), this),
			opponent->attacker->unregister_npc.end()
		);
	}

	this->damagelist.clear();
	this->totaldamage = 0;

	if (show)
	{
		PacketBuilder builder(PACKET_NPC, PACKET_SPEC, 18);
		builder.AddShort(0); // killer pid
		builder.AddChar(0); // killer direction
		builder.AddShort(this->index);
		builder.AddShort(0); // dropped item uid
		builder.AddShort(0); // dropped item id
		builder.AddChar(this->x);
		builder.AddChar(this->y);
		builder.AddInt(0); // dropped item amount
		builder.AddThree(this->hp); // damage

		UTIL_FOREACH(this->map->characters, character)
		{
			if (character->InRange(this))
			{
				character->Send(builder);
			}
		}
	}
}

void NPC::Attack(Character *target)
{
	int amount = util::rand(this->Data()->mindam, this->Data()->maxdam + static_cast<int>(this->map->world->config["NPCAdjustMaxDam"]));
	double rand = util::rand(0.0, 1.0);
	// Checks if target is facing you
	bool critical = std::abs(int(target->direction) - this->direction) != 2 || rand < static_cast<double>(this->map->world->config["CriticalRate"]);

	std::unordered_map<std::string, double> formula_vars;

	this->FormulaVars(formula_vars);
	target->FormulaVars(formula_vars, "target_");
	formula_vars["modifier"] = 1.0 / static_cast<double>(this->map->world->config["MobRate"]);
	formula_vars["damage"] = amount;
	formula_vars["critical"] = critical;

	amount = rpn_eval(rpn_parse(this->map->world->formulas_config["damage"]), formula_vars);
	double hit_rate = rpn_eval(rpn_parse(this->map->world->formulas_config["hit_rate"]), formula_vars);

	if (rand > hit_rate)
	{
		amount = 0;
	}

	amount = std::max(amount, 0);

	int limitamount = std::min(amount, int(target->hp));

	if (this->map->world->config["LimitDamage"])
	{
		amount = limitamount;
	}

	target->hp -= limitamount;
	if (target->party)
	{
		target->party->UpdateHP(target);
	}

	int xdiff = this->x - target->x;
	int ydiff = this->y - target->y;

	if (std::abs(xdiff) > std::abs(ydiff))
	{
		if (xdiff < 0)
		{
			this->direction = DIRECTION_RIGHT;
		}
		else
		{
			this->direction = DIRECTION_LEFT;
		}
	}
	else
	{
		if (ydiff < 0)
		{
			this->direction = DIRECTION_DOWN;
		}
		else
		{
			this->direction = DIRECTION_UP;
		}
	}

	PacketBuilder builder(PACKET_NPC, PACKET_PLAYER, 18);
	builder.AddByte(255);
	builder.AddChar(this->index);
	builder.AddChar(1 + (target->hp == 0));
	builder.AddChar(this->direction);
	builder.AddShort(target->player->id);
	builder.AddThree(amount);
	builder.AddThree(int(double(target->hp) / double(target->maxhp) * 100.0));
	builder.AddByte(255);
	builder.AddByte(255);

	UTIL_FOREACH(this->map->characters, character)
	{
		if (character == target || !character->InRange(target))
		{
			continue;
		}

		character->Send(builder);
	}

	int rechp = int(target->maxhp * static_cast<double>(this->map->world->config["DeathRecover"]));

	if (target->hp == 0)
	{
		builder.AddShort(rechp);
	}
	else
	{
		builder.AddShort(target->hp);
	}
	builder.AddShort(target->tp);

	target->Send(builder);

	if (target->hp == 0)
	{
		target->hp = rechp;

		if (this->map->world->config["Deadly"])
		{
			target->DropAll(0);
		}

		target->map->Leave(target, WARP_ANIMATION_NONE, true);

		target->nowhere = true;
		target->map = this->map->world->GetMap(target->SpawnMap());
		target->mapid = target->SpawnMap();
		target->x = target->SpawnX();
		target->y = target->SpawnY();

		PacketReader reader("");

		target->player->client->queue.AddAction(PacketReader(std::array<char, 2>{
			{char(PACKET_INTERNAL_NULL), char(PACKET_INTERNAL)}
		}.data()), 1.5);

		target->player->client->queue.AddAction(PacketReader(std::array<char, 2>{
			{char(PACKET_INTERNAL_WARP), char(PACKET_INTERNAL)}
		}.data()), 0.0);
	}
}

#define v(x) vars[prefix + #x] = x;
#define vv(x, n) vars[prefix + n] = x;
#define vd(x) vars[prefix + #x] = data->x;

void NPC::FormulaVars(std::unordered_map<std::string, double> &vars, std::string prefix)
{
	ENF_Data *data = this->Data();
	vv(1, "npc") v(hp) vv(data->hp, "maxhp")
	vd(mindam) vd(maxdam)
	vd(accuracy) vd(evade) vd(armor)
	v(x) v(y) v(direction) vv(map->id, "mapid")
}

#undef vd
#undef vv
#undef v

NPC::~NPC()
{
	UTIL_FOREACH(this->map->characters, character)
	{
		if (character->npc == this)
		{
			character->npc = 0;
			character->npc_type = ENF::NPC;
		}
	}
}
