#ifndef EOSERV_HPP_INCLUDED
#define EOSERV_HPP_INCLUDED

#include <list>
#include <map>
#include <ctime>
#include <string>

class World;
class Player;
class Character;
class Guild;
class Party;
class NPC;
class Map;
class ActionQueue;

#include "eoclient.hpp"
#include "database.hpp"
#include "util.hpp"

const int ADMIN_PLAYER = 0;
const int ADMIN_GUIDE = 1;
const int ADMIN_GUARDIAN = 2;
const int ADMIN_GM = 3;
const int ADMIN_HGM = 4;

class World
{
	private:
		World();

	public:
		std::list<Character *> characters;
		std::list<Guild *> guilds;
		std::list<Party *> partys;
		std::list<NPC *> npcs;
		std::map<int, Map *> maps;

		World(util::array<std::string, 5> dbinfo);

		void Login(Character *);
		void Logout(Character *);

		void Msg(Character *from, std::string message);

		void Reboot();
		void Reboot(int seconds, std::string reason);
};

class Map
{
	public:
		int id;
		int width;
		int height;
		std::list<Player *> players;
		std::list<NPC *> npcs;

		void Msg(Character *from, std::string message);
};

class Player
{
	public:
		bool online;
		unsigned int id;
		std::string username;
		std::string password;

		Player(std::string username);

		std::list<Character *> characters;
		Character *character;

		static bool ValidName(std::string username);
		static Player *Login(std::string username, std::string password);
		static bool Create(std::string username, std::string password, std::string fullname, std::string location, std::string email, std::string computer, std::string hdid);
		static bool Exists(std::string username);
		bool AddCharacter(std::string name, int gender, int hairstyle, int haircolor, int race);

		EOClient *client;

		~Player();
};

class Character
{
	public:
		bool online;
		unsigned int id;
		int admin;
		std::string name;
		std::string title;
		int clas;
		int gender;
		int race;
		int hairstyle, haircolor;
		std::string home;
		std::string partner;
		int map, x, y;
		int spawnmap, spawnx, spawny;
		int level, exp;
		int hp, tp, maxhp, maxtp;
		int str, intl, wis, agi, con, cha;
		int statpoints, skillpoints;
		int weight, maxweight;
		int karma;
		bool sitting, visible;
		int bankmax;
		int goldbank;
		int usage;

		enum EquipLocation
		{

		};

		std::list<std::pair<int,int> > inventory;
		std::list<std::pair<int,int> > bank;
		util::array<int, 15> paperdoll;
		std::list<std::pair<int,int> > spells;

		Character(std::string name);

		static bool ValidName(std::string name);
		static Character *Create(Player *, std::string name, int gender, int hairstyle, int haircolor, int race);

		void Msg(Character *from, std::string message);

		~Character();

		Player *player;
		Guild *guild;
		char guild_rank;
		Party *party;
};

class NPC
{
	public:
		int type;
		int x, y;
		bool attack;
		int hp, maxhp;
		std::map<Character *, int> damagelist;
		Player *owner;

		Map *map;
};

class Guild
{
	public:
		std::string tag;
		std::string name;
		std::list<Character *> members;
		std::map<std::string, int> ranks;
		std::time_t created;

		void Msg(Character *from, std::string message);
};

class Party
{
	public:
		Party(Character *host, Character *other);

		Character *host;
		std::list<Character *> members;

		void Msg(Character *from, std::string message);
		void Join(Character *);
		void Part(Character *);
};

#endif // EOSERV_HPP_INCLUDED
