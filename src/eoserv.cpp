
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

Database eoserv_db;
World *the_world;
EIF *eoserv_items;
ENF *eoserv_npcs;
ESF *eoserv_spells;
ECF *eoserv_classes;
Config eoserv_config; // assigned to later
Config admin_config("admin.ini");

// TODO: Clean up these functions
std::string ItemSerialize(std::list<std::pair<int,int> > list)
{
	std::string serialized;
	serialized.reserve(list.size()*10); // Reserve some space to stop some mass-reallocations
	UTIL_FOREACH(list, item)
	{
		serialized.append(static_cast<std::string>(util::variant(item.first)));
		serialized.append(",");
		serialized.append(static_cast<std::string>(util::variant(item.second)));
		serialized.append(";");
	}
	serialized.reserve(0); // Clean up the reserve to save memory
	return serialized;
}

std::list<std::pair<int,int> > ItemUnserialize(std::string serialized)
{
	std::list<std::pair<int,int> > list;
	std::size_t p = 0;
	std::size_t lastp = std::numeric_limits<std::size_t>::max();
	while ((p = serialized.find_first_of(';', p+1)) != std::string::npos)
	{
		std::string part = serialized.substr(lastp+1, p-lastp-1);
		std::size_t pp = 0;
		pp = part.find_first_of(',', 0);
		if (pp == std::string::npos)
		{
			continue;
		}
		int one = static_cast<int>(util::variant(part.substr(0, pp)));
		int two = static_cast<int>(util::variant(part.substr(pp+1)));

		list.push_back(std::make_pair(one,two));
		lastp = p;
	}
	return list;
}
std::string DollSerialize(util::array<int, 15> list)
{
	std::string serialized;
	serialized.reserve(15*5); // Reserve some space to stop some mass-reallocations
	for (int i = 0; i < 15; ++i)
	{
		serialized.append(static_cast<std::string>(util::variant(list[i])));
		serialized.append(",");
	}
	serialized.reserve(0); // Clean up the reserve to save memory
	return serialized;
}

util::array<int, 15> DollUnserialize(std::string serialized)
{
	util::array<int, 15> list;
	// Initialize all paperdoll entrys to 0 in case some are missing in the serialized value
	for (int i = 0; i < 15; ++i)
	{
		list[i] = 0;
	}
	std::size_t p = 0;
	std::size_t lastp = std::numeric_limits<std::size_t>::max();
	int i = 0;
	while ((p = serialized.find_first_of(',', p+1)) != std::string::npos)
	{
		list[i++] = static_cast<int>(util::variant(serialized.substr(lastp+1, p-lastp-1)));
		lastp = p;
	}
	return list;
}

#include "eoserv/character.cpp"
#include "eoserv/guild.cpp"
#include "eoserv/map.cpp"
#include "eoserv/npc.cpp"
#include "eoserv/party.cpp"
#include "eoserv/player.cpp"
#include "eoserv/world.cpp"
