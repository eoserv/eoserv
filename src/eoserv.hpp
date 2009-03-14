#ifndef EOSERV_HPP_INCLUDED
#define EOSERV_HPP_INCLUDED

#include <list>
#include <vector>
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

#include "database.hpp"
#include "util.hpp"
#include "config.hpp"

std::string ItemSerialize(std::list<std::pair<int,int> > list);
std::list<std::pair<int,int> > ItemUnserialize(std::string serialized);

extern Config admin_config;

class World
{
	private:
		World();

	public:
		std::list<Character *> characters;
		std::list<Guild *> guilds;
		std::list<Party *> partys;
		std::list<NPC *> npcs;
		std::vector<Map *> maps;

		World(util::array<std::string, 5> dbinfo, Config);

		void Login(Character *);
		void Logout(Character *);

		void Msg(Character *from, std::string message);
		void AdminMsg(Character *from, std::string message);
		void AnnounceMsg(Character *from, std::string message);

		void Reboot();
		void Reboot(int seconds, std::string reason);

		void Kick(Character *from, Character *victim);

		Character *GetCharacter(std::string name);
};

#include "eoclient.hpp"

class Map
{
	public:
		int id;
		char rid[4];
		int width;
		int height;
		std::string filename;
		std::list<Character *> characters;
		std::list<NPC *> npcs;

		Map(int id);

		void Enter(Character *);
		void Leave(Character *);

		void Msg(Character *from, std::string message);
		void Walk(Character *from, int direction);
		void Attack(Character *from, int direction);
		void Face(Character *from, int direction);
		void Sit(Character *from);
		void Stand(Character *from);
		void Emote(Character *from, int emote);
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
		void ChangePass(std::string password);
		static bool Online(std::string username);

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
		std::string home;
		std::string partner;
		int clas;
		int gender;
		int race;
		int hairstyle, haircolor;
		int mapid, x, y, direction;
		int spawnmap, spawnx, spawny;
		int level, exp;
		int hp, tp;
		int str, intl, wis, agi, con, cha;
		int statpoints, skillpoints;
		int weight, maxweight;
		int karma;
		bool sitting, visible;
		int bankmax;
		int goldbank;
		int usage;

		int maxsp;
		int maxhp, maxtp;
		int accuracy, evade, armor;
		int mindam, maxdam;

		enum EquipLocation
		{
			Boots,
			Accessory,
			Gloves,
			Belt,
			Armor,
			Necklace,
			Hat,
			Shield,
			Weapon,
			Ring1,
			Ring2,
			Armlet1,
			Armlet2,
			Bracer1,
			Bracer2
		};

		std::list<std::pair<int,int> > inventory;
		std::list<std::pair<int,int> > bank;
		util::array<int, 15> paperdoll;
		std::list<std::pair<int,int> > spells;

		Character(std::string name);

		static bool ValidName(std::string name);
		static Character *Create(Player *, std::string name, int gender, int hairstyle, int haircolor, int race);
		static void Delete(std::string name);

		void Msg(Character *from, std::string message);
		void Walk(int direction);
		void Attack(int direction);
		void Sit();
		void Stand();
		void Emote(int emote);
		int HasItem(int item);
		void AddItem(int item, int amount);
		void DelItem(int item, int amount);
		bool Unequip(int item);
		bool Equip(int item);
		bool InRange(Character *);

		~Character();

		Player *player;
		Guild *guild;
		char guild_rank;
		Party *party;
		Map *map;
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
