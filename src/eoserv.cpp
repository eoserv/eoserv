
#include "eoserv.hpp"

#include <string>
#include <list>
#include <vector>
#include <ctime>
#include <cmath>
#include <limits>
#include <algorithm>

#include "util.hpp"
#include "eoconst.hpp"
#include "eodata.hpp"
#include "database.hpp"
#include "hash.hpp"
#include "timer.hpp"

Database eoserv_db;
World *the_world;
EIF *eoserv_items;
ENF *eoserv_npcs;
ESF *eoserv_spells;
ECF *eoserv_classes;

// assigned to in World::World()
Config eoserv_config;
Config admin_config;
Config drops_config;
Config shops_config;

void world_spawn_npcs(void *world_void);

// TODO: Clean up these functions
std::string ItemSerialize(std::list<Character_Item> list)
{
	std::string serialized;
	serialized.reserve(list.size()*10); // Reserve some space to stop some mass-reallocations
	UTIL_LIST_FOREACH_ALL(list, Character_Item, item)
	{
		serialized.append(util::to_string(item.id));
		serialized.append(",");
		serialized.append(util::to_string(item.amount));
		serialized.append(";");
	}
	serialized.reserve(0); // Clean up the reserve to save memory
	return serialized;
}

std::list<Character_Item> ItemUnserialize(std::string serialized)
{
	std::list<Character_Item> list;
	std::size_t p = 0;
	std::size_t lastp = std::numeric_limits<std::size_t>::max();
	Character_Item newitem;

	while ((p = serialized.find_first_of(';', p+1)) != std::string::npos)
	{
		std::string part = serialized.substr(lastp+1, p-lastp-1);
		std::size_t pp = 0;
		pp = part.find_first_of(',', 0);
		if (pp == std::string::npos)
		{
			continue;
		}
		newitem.id = util::to_int(part.substr(0, pp));
		newitem.amount = util::to_int(part.substr(pp+1));

		list.push_back(newitem);

		lastp = p;
	}

	return list;
}

std::string DollSerialize(util::array<int, 15> list)
{
	std::string serialized;

	serialized.reserve(15*5); // Reserve some space to stop some mass-reallocations

	UTIL_ARRAY_FOREACH_ALL(list, int, 15, item)
	{
		serialized.append(util::to_string(item));
		serialized.append(",");
	}

	serialized.reserve(0); // Clean up the reserve to save memory

	return serialized;
}

util::array<int, 15> DollUnserialize(std::string serialized)
{
	util::array<int, 15> list(0);
	std::size_t p = 0;
	std::size_t lastp = std::numeric_limits<std::size_t>::max();
	int i = 0;
	while ((p = serialized.find_first_of(',', p+1)) != std::string::npos)
	{
		list[i++] = util::to_int(serialized.substr(lastp+1, p-lastp-1));
		lastp = p;
	}
	return list;
}

// Stop doxygen generating a gigantic include graph
#ifndef DOXYGEN

#include "eoserv/character.cpp"
#include "eoserv/guild.cpp"
#include "eoserv/map.cpp"
#include "eoserv/npc.cpp"
#include "eoserv/party.cpp"
#include "eoserv/player.cpp"
#include "eoserv/world.cpp"

#endif // DOXYGEN
