#ifndef EOSERV_HPP_INCLUDED
#define EOSERV_HPP_INCLUDED

#include <list>
#include <map>
#include <ctime>
#include <string>

class World;
class Player;
class Guild;
class Party;
class NPC;
class Map;
class ActionQueue;

#include "eoclient.hpp"

class World
{
	public:
		std::list<Player *> players;
		std::list<Guild *> guilds;
		std::list<Party *> partys;
		std::list<NPC *> npcs;
		std::map<int, Map *> maps;

		void Msg(Player *from, std::string message);
};

class Map
{
	public:
		int id;
		int width;
		int height;
		std::list<Player *> players;
		std::list<NPC *> npcs;

		World *world;

		void Msg(Player *from, std::string message);
};

class Player
{
	public:
		int admin;
		std::string name;
		int x,y;
		int level, exp;
		int hp, tp, sp, maxhp, maxtp, maxsp;
		int str, intl, wis, agi, cha;

		std::list<std::pair<int,int> > inventory;
		std::list<int> spells;
		std::list<int> skills;

		EOClient *client;
		World *world;
		Map *map;
		Guild *guild;
		int guild_rank;
		Party *party;

		void Msg(Player *from, std::string message);
};

class NPC
{
	public:
		int type;
		int x,y;
		bool attack;
		int hp, maxhp;
		std::map<Player *, int> damagelist;
		Player *owner;

		Map *map;
};

class Guild
{
	public:
		std::string tag;
		std::string name;
		std::list<Player *> members;
		std::map<std::string, int> ranks;
		std::time_t created;

		void Msg(Player *from, std::string message);
};

class Party
{
	public:
		Party(Player *host, Player *);

		Player *host;
		std::list<Player *> members;

		void Msg(Player *from, std::string message);
		void JoinPlayer(Player *);
		void PartPlayer(Player *);
};

#endif // EOSERV_HPP_INCLUDED
