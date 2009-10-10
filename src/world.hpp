
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef WORLD_HPP_INCLUDED
#define WORLD_HPP_INCLUDED

#include <vector>
#include <string>

class World;

struct Board_Post;
struct Board;

#include "character.hpp"
#include "guild.hpp"
#include "party.hpp"
#include "map.hpp"

#include "eoserver.hpp"
#include "eoconst.hpp"
#include "timer.hpp"
#include "util.hpp"
#include "database.hpp"
#include "eodata.hpp"
#include "config.hpp"
#include "socket.hpp"
#include "hook.hpp"

SCRIPT_STRUCT_VALUE(Board_Post)
{
	short id;
	std::string author;
	int author_admin;
	std::string subject;
	std::string body;
	double time;

	SCRIPT_REGISTER(Board_Post)
	{
		Board_Post *inst = reinterpret_cast<Board_Post *>(std::malloc(sizeof(inst)));
		SCRIPT_REGISTER_VARIABLE_OFFSET("int16", "id", instance_offsetof(inst, id));
		SCRIPT_REGISTER_VARIABLE_OFFSET("string", "author", instance_offsetof(inst, author));
		SCRIPT_REGISTER_VARIABLE_OFFSET("int", "author_admin", instance_offsetof(inst, author_admin));
		SCRIPT_REGISTER_VARIABLE_OFFSET("string", "subject", instance_offsetof(inst, subject));
		SCRIPT_REGISTER_VARIABLE_OFFSET("string", "body", instance_offsetof(inst, body));
		SCRIPT_REGISTER_VARIABLE_OFFSET("double", "time", instance_offsetof(inst, time));
		std::free(inst);
	}
};

SCRIPT_STRUCT_REF(Board)
{
	short last_id;
	std::list<Board_Post *> posts;

	Board() : last_id(0) { }

	~Board();

	SCRIPT_REGISTER(Board)
	{
		Board *inst = reinterpret_cast<Board *>(std::malloc(sizeof(inst)));
		SCRIPT_REGISTER_VARIABLE_OFFSET("int16", "last_id", instance_offsetof(inst, last_id));
		SCRIPT_REGISTER_VARIABLE_OFFSET("_list_Board_Post_ptr", "posts", instance_offsetof(inst, posts));
		std::free(inst);
	}
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

		HookManager *hookmanager;

		EIF *eif;
		ENF *enf;
		ESF *esf;
		ECF *ecf;

		Config config;
		Config admin_config;
		Config drops_config;
		Config shops_config;
		Config arenas_config;

		std::vector<Character *> characters;
		std::vector<Guild *> guilds;
		std::vector<Party *> parties;
		std::vector<Map *> maps;

		util::array<Board *, 8> boards;

		util::array<int, 254> exp_table;

		World(util::array<std::string, 5> dbinfo, const Config &eoserv_config, const Config &admin_config);

		int GenerateCharacterID();
		int GeneratePlayerID();

		void Login(Character *);
		void Logout(Character *);

		void Msg(Character *from, std::string message);
		void AdminMsg(Character *from, std::string message, int minlevel = ADMIN_GUARDIAN);
		void AnnounceMsg(Character *from, std::string message);
		void ServerMsg(std::string message);

		void Reboot();
		void Reboot(int seconds, std::string reason);

		void Kick(Character *from, Character *victim, bool announce = true);
		void Jail(Character *from, Character *victim, bool announce = true);
		void Ban(Character *from, Character *victim, int duration, bool announce = true);

		int CheckBan(const std::string *username, const IPAddress *address, const int *hdid);

		Character *GetCharacter(std::string name);
		Character *GetCharacterPID(unsigned int id);
		Character *GetCharacterCID(unsigned int id);

		Map *GetMap(short id);

		bool CharacterExists(std::string name);
		Character *CreateCharacter(Player *, std::string name, Gender, int hairstyle, int haircolor, Skin);
		void DeleteCharacter(std::string name);

		Player *Login(std::string username, std::string password);
		bool CreatePlayer(std::string username, std::string password, std::string fullname, std::string location, std::string email, std::string computer, std::string hdid, std::string ip);
		bool PlayerExists(std::string username);
		bool PlayerOnline(std::string username);

		bool PKExcept(const Map *map);
		bool PKExcept(int mapid);

		~World();
};

#endif // WORLD_HPP_INCLUDED
