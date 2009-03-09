
#include "eoserv.hpp"

#include <string>
#include <list>
#include <vector>
#include <ctime>
#include <cmath>

#include "util.hpp"
#include "eoconst.hpp"
#include "eodata.hpp"
#include "database.hpp"

Database eoserv_db;
World *the_world;
EIF *eoserv_items;
ENF *eoserv_npcs;
ESF *eoserv_spells;
ECF *eoserv_classes;

#include "eoserv/character.cpp"
#include "eoserv/guild.cpp"
#include "eoserv/map.cpp"
#include "eoserv/npc.cpp"
#include "eoserv/party.cpp"
#include "eoserv/player.cpp"
#include "eoserv/world.cpp"
