
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "npc.hpp"

#include <string>
#include <vector>
#include <set>
#include <cstdio>
#include <cmath>

#include "util.hpp"

static const double speed_table[8] = {0.9, 0.6, 1.3, 1.9, 3.7, 7.5, 15.0, 0.0};

NPC::NPC(Map *map, short id, unsigned char x, unsigned char y, unsigned char spawn_type, short spawn_time, unsigned char index)
{
	this->map = map;
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

	this->data = map->world->enf->Get(id);

	if (spawn_type == 7)
	{
		this->direction = static_cast<Direction>(spawn_time & 0x03);
		this->spawn_time = 0;
	}

	Config::iterator drops = map->world->drops_config.find(util::to_string(this->id));
	if (drops != map->world->drops_config.end())
	{
		std::vector<std::string> parts = util::explode(',', static_cast<std::string>((*drops).second));

		if (parts.size() > 1)
		{
			if (parts.size() % 4 != 0)
			{
				std::fprintf(stderr, "WARNING: skipping invalid drop data for NPC #%i\n", id);
				return;
			}

			this->drops.resize(parts.size() / 4);

			for (std::size_t i = 0; i < parts.size(); i += 4)
			{
				NPC_Drop drop;

				drop.id = util::to_int(parts[i]);
				drop.min = util::to_int(parts[i+1]);
				drop.max = util::to_int(parts[i+2]);
				drop.chance = util::to_float(parts[i+3]);

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
				std::fprintf(stderr, "WARNING: skipping invalid trade shop data for NPC #%i\n", id);
				return;
			}

			this->shop_trade.resize(parts.size() / 3);

			for (std::size_t i = 0; i < parts.size(); i += 3)
			{
				NPC_Shop_Trade_Item item;
				item.id = util::to_int(parts[i]);
				item.buy = util::to_int(parts[i+1]);
				item.sell = util::to_int(parts[i+2]);

				if (item.buy != 0 && item.sell != 0 && item.sell > item.buy)
				{
					std::fprintf(stderr, "WARNING: item #%i (NPC #%i) has a higher sell price than buy price.\n", item.id, id);
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
				std::fprintf(stderr, "WARNING: skipping invalid craft shop data for NPC #%i\n", id);
				return;
			}

			this->shop_craft.resize(parts.size() / 9);

			for (std::size_t i = 0; i < parts.size(); i += 9)
			{
				NPC_Shop_Craft_Item item;
				std::vector<NPC_Shop_Craft_Ingredient> ingredients(4);
				NPC_Shop_Craft_Ingredient ingredient;

				item.id = util::to_int(parts[i]);

				for (int ii = 0; ii < 4; ++ii)
				{
					ingredient.id = util::to_int(parts[i+1+ii*2]);
					ingredient.amount = util::to_int(parts[i+2+ii*2]);
					ingredients[ii] = ingredient;
				}

				item.ingredients = ingredients;

				this->shop_craft[i/9] = item;
			}
		}
	}
}

void NPC::Spawn()
{
	if (this->spawn_type < 7)
	{
		bool found = false;
		for (int i = 0; i < 200; ++i)
		{
			this->x = util::rand(this->spawn_x-2, this->spawn_x+2);
			this->y = util::rand(this->spawn_y-2, this->spawn_y+2);

			if (this->map->Walkable(this->x, this->y, true) && (i > 100 || !this->map->Occupied(this->x, this->y, Map::NPCOnly)))
			{
				this->direction = static_cast<Direction>(util::rand(0,3));
				found = true;
				break;
			}
		}

		if (!found)
		{
			std::fprintf(stderr, "Warning: An NPC on map %i at %i,%i is being placed by linear scan of spawn area\n", this->map->id, this->spawn_x, this->spawn_y);
			for (this->x = this->spawn_x-2; this->x <= spawn_x+2; ++this->x)
			{
				for (this->y = this->spawn_y-2; this->y <= this->spawn_y+2; ++this->y)
				{
					if (this->map->Walkable(this->x, this->y, true))
					{
						std::fprintf(stderr, "Placed at valid location: %i,%i\n", this->x, this->y);
						found = true;
						goto end_linear_scan;
					}
				}
			}
		}
		end_linear_scan:

		if (!found)
		{
			std::fputs("Error: NPC couldn't spawn anywhere valid!\n", stderr);
		}
	}

	this->alive = true;
	this->hp = this->data->hp;
	this->last_act = Timer::GetTime();
	this->act_speed = speed_table[this->spawn_type];

	PacketBuilder builder(PACKET_APPEAR, PACKET_REPLY);
	builder.AddChar(0);
	builder.AddByte(255);
	builder.AddChar(this->index);
	builder.AddShort(this->id);
	builder.AddChar(this->x);
	builder.AddChar(this->y);
	builder.AddChar(this->direction);

	UTIL_VECTOR_FOREACH_ALL(this->map->characters, Character *, character)
	{
		if (character->InRange(this))
		{
			character->player->client->SendBuilder(builder);
		}
	}
}

void NPC::Act()
{
	this->last_act += double(util::rand(int(this->act_speed * 750.0), int(this->act_speed * 1250.0))) / 1000.0;

	if (this->spawn_type == 7)
	{
		return;
	}

	Character *attacker = 0;
	unsigned char attacker_distance = static_cast<int>(this->map->world->config["NPCChaseDistance"]);
	unsigned short attacker_damage = 0;
	int npccoordsum = this->x + this->y;

	UTIL_LIST_IFOREACH_ALL(this->damagelist, NPC_Opponent, opponent)
	{
		if (opponent->attacker->map != this->map || opponent->last_hit < Timer::GetTime() - static_cast<double>(this->map->world->config["NPCBoredTimer"]))
		{
			continue;
		}

		int distance = std::abs(opponent->attacker->x + opponent->attacker->y - npccoordsum);

		if ((distance < attacker_distance) || (distance == attacker_distance && opponent->damage > attacker_damage))
		{
			attacker = opponent->attacker;
			attacker_damage = opponent->damage;
			attacker_distance = distance;
		}
	}

	if (this->data->type == ENF::Aggressive && !attacker)
	{
		Character *closest = 0;
		unsigned char closest_distance = static_cast<int>(this->map->world->config["SeeDistance"]);

		UTIL_VECTOR_FOREACH_ALL(this->map->characters, Character *, character)
		{
			int distance = std::abs(character->x + character->y - npccoordsum);

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

void NPC::Damage(Character *from, int amount)
{
	double droprate = static_cast<double>(this->map->world->config["DropRate"]) / 100.0;
	double exprate = static_cast<double>(this->map->world->config["ExpRate"]) / 100.0;
	int sharemode = static_cast<int>(this->map->world->config["ShareMode"]);
	int partysharemode = static_cast<int>(this->map->world->config["PartyShareMode"]);
	std::set<Party *> parties;
	PacketBuilder builder;

	amount = std::min(this->hp, amount);
	this->hp -= amount;
	this->totaldamage += amount;

	NPC_Opponent opponent;
	bool found = false;

	UTIL_LIST_IFOREACH_ALL(this->damagelist, NPC_Opponent, checkopp)
	{
		if (checkopp->attacker == from)
		{
			found = true;
			checkopp->damage += amount;
			checkopp->last_hit = Timer::GetTime();
		}
	}

	if (!found)
	{
		opponent.attacker = from;
		opponent.damage = amount;
		opponent.last_hit = Timer::GetTime();
		this->damagelist.push_back(opponent);
		opponent.attacker->unregister_npc.push_back(this);
	}

	if (this->hp > 0)
	{
		builder.SetID(PACKET_NPC, PACKET_REPLY);
		builder.AddShort(from->player->id);
		builder.AddChar(from->direction);
		builder.AddShort(this->index);
		builder.AddThree(amount);
		builder.AddShort(int(double(this->hp) / double(this->data->hp) * 100.0));
		builder.AddChar(1); // ?

		UTIL_VECTOR_FOREACH_ALL(this->map->characters, Character *, character)
		{
			if (character->InRange(this))
			{
				character->player->client->SendBuilder(builder);
			}
		}
	}
	else
	{
		int most_damage_counter = 0;
		Character *most_damage = 0;
		this->alive = false;
		this->dead_since = int(Timer::GetTime());

		std::vector<NPC_Drop> drops;
		NPC_Drop *drop = 0;
		UTIL_VECTOR_FOREACH_ALL(this->drops, NPC_Drop, checkdrop)
		{
			if ((double(util::rand(0,10000)) / 100.0) < checkdrop.chance * droprate)
			{
				drops.push_back(checkdrop);
			}
		}

		if (drops.size() > 0)
		{
			drop = &drops[util::rand(0, drops.size()-1)];
		}

		if (sharemode == 1)
		{
			UTIL_LIST_FOREACH_ALL(this->damagelist, NPC_Opponent, opponent)
			{
				if (opponent.damage > most_damage_counter)
				{
					most_damage_counter = opponent.damage;
					most_damage = opponent.attacker;
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
			Map_Item newitem = {dropuid, dropid, dropamount, this->x, this->y, 0, Timer::GetTime() + static_cast<int>(this->map->world->config["ProtectNPCDrop"])};
			this->map->items.push_back(newitem);

			// Selects a random number between 0 and maxhp, and decides the winner based on that
			switch (sharemode)
			{
				case 0:
					this->map->items.back().owner = from->player->id;
					break;

				case 1:
					this->map->items.back().owner = most_damage->player->id;
					break;

				case 2:
				{
					int rewarded_hp = util::rand(0, this->data->hp);
					int count_hp = 0;
					UTIL_LIST_FOREACH_ALL(this->damagelist, NPC_Opponent, opponent)
					{
						if (opponent.attacker->InRange(this))
						{
							if (rewarded_hp >= count_hp && rewarded_hp < opponent.damage)
							{
								this->map->items.back().owner = opponent.attacker->player->id;
								break;
							}

							count_hp += opponent.damage;
						}
					}
				}
					break;

				case 3:
				{
					int rand = util::rand(0, this->damagelist.size());
					int i = 0;
					UTIL_LIST_FOREACH_ALL(this->damagelist, NPC_Opponent, opponent)
					{
						if (opponent.attacker->InRange(this))
						{
							if (rand == i++)
							{
								this->map->items.back().owner = opponent.attacker->player->id;
								break;
							}
						}
					}
				}
					break;
			}
		}

		UTIL_VECTOR_FOREACH_ALL(this->map->characters, Character *, character)
		{
			std::list<NPC_Opponent>::iterator findopp;
			found = false;
			UTIL_LIST_IFOREACH_ALL(this->damagelist, NPC_Opponent, checkopp)
			{
				if (checkopp->attacker == character)
				{
					found = true;
					findopp = checkopp;
				}
			}

			if (found || character->InRange(this))
			{
				bool level_up = false;

				builder.Reset();

				builder.SetID(PACKET_NPC, PACKET_SPEC);

				if (this->data->exp != 0)
				{
					if (found)
					{
						int reward;
						switch (sharemode)
						{
							case 0:
								if (character == from)
								{
									reward = std::ceil(double(this->data->exp) * exprate);

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
									reward = std::ceil(double(this->data->exp) * exprate);

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
								reward = std::ceil(double(this->data->exp) * exprate * (double(findopp->damage) / double(this->totaldamage)));

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
								reward = std::ceil(double(this->data->exp) * exprate * (double(this->damagelist.size()) / 1.0));

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

						if (character->level < static_cast<int>(this->map->world->config["MaxLevel"]) && character->exp >= this->map->world->exp_table[character->level+1])
						{
							level_up = true;
							++character->level;
							character->statpoints += static_cast<int>(this->map->world->config["StatPerLevel"]);
							character->skillpoints += static_cast<int>(this->map->world->config["SkillPerLevel"]);
							character->CalculateStats();
						}

						if (level_up)
						{
							builder.SetID(PACKET_NPC, PACKET_ACCEPT);
						}
					}
				}

				builder.AddShort(from->player->id);
				builder.AddChar(from->direction);
				builder.AddShort(this->index);
				builder.AddShort(dropuid);
				builder.AddShort(dropid);
				builder.AddChar(this->x);
				builder.AddChar(this->y);
				builder.AddInt(dropamount);
				builder.AddThree(amount);

				if ((sharemode == 0 && character == from) || (sharemode != 0 && found))
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

				character->player->client->SendBuilder(builder);
			}
		}

		UTIL_SET_FOREACH_ALL(parties, Party *, party)
		{
			party->ShareEXP(party->temp_expsum, partysharemode, this->map);
			party->temp_expsum = 0;
		}

		UTIL_LIST_FOREACH_ALL(this->damagelist, NPC_Opponent, opponent)
		{
			std::list<NPC *>::iterator findnpc = std::find(opponent.attacker->unregister_npc.begin(), opponent.attacker->unregister_npc.end(), this);

			if (findnpc != opponent.attacker->unregister_npc.end())
			{
				opponent.attacker->unregister_npc.erase(findnpc);
			}
		}

		this->damagelist.clear();
		this->totaldamage = 0;
	}
}

void NPC::Attack(Character *target)
{
	double mobrate = static_cast<double>(this->map->world->config["MobRate"]) / 100.0;

	int amount = util::rand(this->data->mindam, this->data->maxdam + static_cast<int>(this->map->world->config["NPCAdjustMaxDam"]));

	int hit_rate = 120;
	bool critical = true;

	if ((target->direction == DIRECTION_UP && this->direction == DIRECTION_DOWN)
	 || (target->direction == DIRECTION_RIGHT && this->direction == DIRECTION_LEFT)
	 || (target->direction == DIRECTION_DOWN && this->direction == DIRECTION_UP)
	 || (target->direction == DIRECTION_LEFT && this->direction == DIRECTION_RIGHT)
	 || target->sitting != SIT_STAND)
	{
		hit_rate -= 40;
	}

	hit_rate += int(double(this->data->accuracy) / 2.0 * mobrate);
	hit_rate -= int(double(target->evade) / 2.0);
	hit_rate = std::min(std::max(hit_rate, 20), 100);

	int origamount = amount;
	amount -= int(double(target->armor) / 3.0);

	amount = std::max(amount, int(std::ceil(double(origamount) * 0.1)));
	amount = std::min(amount, this->hp);

	int rand = util::rand(0, 100);

	if (rand > hit_rate)
	{
		amount = 0;
	}

	if (rand > 92)
	{
		critical = true;
	}

	if (critical)
	{
		amount = int(double(amount) * 1.5);
	}

	amount = std::max(amount, 0);
	amount = std::min(amount, int(target->hp));

	target->hp -= amount;
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

	PacketBuilder builder(PACKET_NPC, PACKET_PLAYER);
	builder.AddByte(255);
	builder.AddChar(this->index);
	builder.AddChar(1 + (target->hp == 0));
	builder.AddChar(this->direction);
	builder.AddShort(target->player->id);
	builder.AddThree(amount);
	builder.AddThree(int(double(target->hp) / double(target->maxhp) * 100.0));
	builder.AddByte(255);
	builder.AddByte(255);

	UTIL_VECTOR_FOREACH_ALL(this->map->characters, Character *, character)
	{
		if (character == target || !character->InRange(target))
		{
			continue;
		}

		character->player->client->SendBuilder(builder);
	}

	builder.AddShort(target->hp);
	builder.AddShort(target->tp);

	target->player->client->SendBuilder(builder);

	if (target->hp == 0)
	{
		target->Warp(target->spawnmap, target->spawnx, target->spawny);
	}
}