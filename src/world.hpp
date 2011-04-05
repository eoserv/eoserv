
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef WORLD_HPP_INCLUDED
#define WORLD_HPP_INCLUDED

#include "fwd/world.hpp"

#include <algorithm>
#include <list>
#include <string>
#include <vector>

#include "config.hpp"
#include "database.hpp"
#include "timer.hpp"

#include "fwd/character.hpp"
#include "fwd/eodata.hpp"
#include "fwd/eoserver.hpp"
#include "fwd/guild.hpp"
#include "fwd/map.hpp"
#include "fwd/party.hpp"
#include "fwd/player.hpp"
#include "fwd/socket.hpp"

struct Board_Post
{
	short id;
	std::string author;
	int author_admin;
	std::string subject;
	std::string body;
	double time;
};

struct Board
{
	int id;
	short last_id;
	std::list<Board_Post *> posts;

	Board(int id_) : id(id_), last_id(0) { }
};

struct Home
{
	std::string id;
	std::string name;
	short map;
	unsigned char x;
	unsigned char y;
	int level;

	Home() : map(1), x(0), y(0), level(-1) { }
};

/**
 * Object which holds and manages all maps and characters on the server, as well as timed events
 * Only one of these should exist per server
 */
class World
{
	protected:
		int last_character_id;

	public:
		Timer timer;

		EOServer *server;
		Database db;

		GuildManager *guildmanager;

		EIF *eif;
		ENF *enf;
		ESF *esf;
		ECF *ecf;

		Config config;
		Config admin_config;
		Config drops_config;
		Config shops_config;
		Config arenas_config;
		Config formulas_config;
		Config home_config;
		Config skills_config;

		std::vector<Character *> characters;
		std::vector<Guild *> guilds;
		std::vector<Party *> parties;
		std::vector<Map *> maps;
		std::vector<Home *> homes;

		std::array<Board *, 8> boards;

		std::array<int, 254> exp_table;

		World(std::array<std::string, 6> dbinfo, const Config &eoserv_config, const Config &admin_config);
		void LoadHome();

		int GenerateCharacterID();
		int GeneratePlayerID();

		void Login(Character *);
		void Logout(Character *);

		void Msg(Character *from, std::string message, bool echo = true);
		void AdminMsg(Character *from, std::string message, int minlevel = ADMIN_GUARDIAN, bool echo = true);
		void AnnounceMsg(Character *from, std::string message, bool echo = true);
		void ServerMsg(std::string message);
		void AdminReport(Character *from, std::string reportee, std::string message);
		void AdminRequest(Character *from, std::string message);

		void Reboot();
		void Reboot(int seconds, std::string reason);

		void Rehash();
		void ReloadPub();

		void Kick(Character *from, Character *victim, bool announce = true);
		void Jail(Character *from, Character *victim, bool announce = true);
		void Ban(Character *from, Character *victim, int duration, bool announce = true);

		int CheckBan(const std::string *username, const IPAddress *address, const int *hdid);

		Character *GetCharacter(std::string name);
		Character *GetCharacterPID(unsigned int id);
		Character *GetCharacterCID(unsigned int id);

		Map *GetMap(short id);
		Home *GetHome(Character *);
		Home *GetHome(std::string);

		bool CharacterExists(std::string name);
		Character *CreateCharacter(Player *, std::string name, Gender, int hairstyle, int haircolor, Skin);
		void DeleteCharacter(std::string name);

		Player *Login(std::string username, std::string password);
		Player *Login(std::string username);
		LoginReply LoginCheck(std::string username, std::string password);
		bool CreatePlayer(std::string username, std::string password, std::string fullname, std::string location, std::string email, std::string computer, std::string hdid, std::string ip);
		bool PlayerExists(std::string username);
		bool PlayerOnline(std::string username);

		bool PKExcept(const Map *map);
		bool PKExcept(int mapid);

		~World();
};

#endif // WORLD_HPP_INCLUDED
