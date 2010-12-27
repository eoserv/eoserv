
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef WORLD_HPP_INCLUDED
#define WORLD_HPP_INCLUDED

#include "stdafx.h"

#include "config.hpp"
#include "database.hpp"
#include "eoconst.hpp"
#include "script.hpp"
#include "timer.hpp"

struct Board_Post : public Shared
{
	short id;
	std::string author;
	int author_admin;
	std::string subject;
	std::string body;
	double time;

	SCRIPT_REGISTER_REF_DF(Board_Post)
		SCRIPT_REGISTER_VARIABLE("int16", id);
		SCRIPT_REGISTER_VARIABLE("string", author);
		SCRIPT_REGISTER_VARIABLE("int", author_admin);
		SCRIPT_REGISTER_VARIABLE("string", subject);
		SCRIPT_REGISTER_VARIABLE("string", body);
		SCRIPT_REGISTER_VARIABLE("double", time);
	SCRIPT_REGISTER_END()
};

struct Board : public Shared
{
	int id;
	short last_id;
	PtrList<Board_Post> posts;

	Board(int id_) : id(id_), last_id(0) { }

	static Board *ScriptFactory(int id) { return new Board(id); }

	SCRIPT_REGISTER_REF(Board)
		SCRIPT_REGISTER_FACTORY("Board @f(int id)", ScriptFactory);

		SCRIPT_REGISTER_VARIABLE("int16", last_id);
		SCRIPT_REGISTER_VARIABLE("PtrList<Board_Post>", posts);
	SCRIPT_REGISTER_END()
};

struct Home : public Shared
{
	std::string id;
	std::string name;
	short map;
	unsigned char x;
	unsigned char y;
	int level;

	Home() : map(1), x(0), y(0), level(-1) { }

	SCRIPT_REGISTER_REF_DF(Home)
		SCRIPT_REGISTER_VARIABLE("string", id);
		SCRIPT_REGISTER_VARIABLE("string", name);
		SCRIPT_REGISTER_VARIABLE("int16", map);
		SCRIPT_REGISTER_VARIABLE("uint8", x);
		SCRIPT_REGISTER_VARIABLE("uint8", y);
		SCRIPT_REGISTER_VARIABLE("int", level);
	SCRIPT_REGISTER_END()
};

/**
 * Object which holds and manages all maps and characters on the server, as well as timed events
 * Only one of these should exist per server
 */
class World : public Shared
{
	protected:
		int last_character_id;

	public:
		Timer timer;

		EOServer *server;
		Database db;

		HookManager *hookmanager;

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

		PtrVector<Character> characters;
		PtrVector<Guild> guilds;
		PtrVector<Party> parties;
		PtrVector<Map> maps;
		PtrVector<Home> homes;

		util::array<Board *, 8> boards;

		util::array<int, 254> exp_table;

		World(util::array<std::string, 6> dbinfo, const Config &eoserv_config, const Config &admin_config);
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
		LoginReply LoginCheck(std::string username, std::string password);
		bool CreatePlayer(std::string username, std::string password, std::string fullname, std::string location, std::string email, std::string computer, std::string hdid, std::string ip);
		bool PlayerExists(std::string username);
		bool PlayerOnline(std::string username);

		bool PKExcept(const Map *map);
		bool PKExcept(int mapid);

		~World();

	SCRIPT_REGISTER_REF(World)
		SCRIPT_REGISTER_VARIABLE("Timer", timer);
		SCRIPT_REGISTER_VARIABLE("EOServer @", server);
		SCRIPT_REGISTER_VARIABLE("Database", db);
		SCRIPT_REGISTER_VARIABLE("EIF @", eif);
		SCRIPT_REGISTER_VARIABLE("ENF @", enf);
		SCRIPT_REGISTER_VARIABLE("ESF @", esf);
		SCRIPT_REGISTER_VARIABLE("ECF @", ecf);
		SCRIPT_REGISTER_VARIABLE("Config", config);
		SCRIPT_REGISTER_VARIABLE("Config", admin_config);
		SCRIPT_REGISTER_VARIABLE("Config", drops_config);
		SCRIPT_REGISTER_VARIABLE("Config", shops_config);
		SCRIPT_REGISTER_VARIABLE("Config", arenas_config);
		SCRIPT_REGISTER_VARIABLE("Config", formulas_config);
		SCRIPT_REGISTER_VARIABLE("Config", home_config);
		SCRIPT_REGISTER_VARIABLE("PtrVector<Character @>", characters);
		SCRIPT_REGISTER_VARIABLE("PtrVector<Guild @>", guilds);
		SCRIPT_REGISTER_VARIABLE("PtrVector<Party @>", parties);
		SCRIPT_REGISTER_VARIABLE("PtrVector<Map @>", maps);
		//SCRIPT_REGISTER_VARIABLE("Array<Board @, 8>", boards);
		//SCRIPT_REGISTER_VARIABLE("Array<int, 254>", exp_table);
		SCRIPT_REGISTER_FUNCTION("void LoadHome()", LoadHome);
		SCRIPT_REGISTER_FUNCTION("int GenerateCharacterID()", GenerateCharacterID);
		SCRIPT_REGISTER_FUNCTION("int GeneratePlayerID()", GeneratePlayerID);
		SCRIPT_REGISTER_FUNCTION_PR("void Login(Character @)", Login, (Character *), void);
		SCRIPT_REGISTER_FUNCTION("void Logout(Character @)", Logout);
		SCRIPT_REGISTER_FUNCTION("void Msg(Character @, string, bool echo)", Msg);
		SCRIPT_REGISTER_FUNCTION("void AdminMsg(Character @, string, int minlevel, bool echo)", AdminMsg);
		SCRIPT_REGISTER_FUNCTION("void AnnounceMsg(Character @, string, bool echo)", AnnounceMsg);
		SCRIPT_REGISTER_FUNCTION("void ServerMsg(string)", ServerMsg);
		SCRIPT_REGISTER_FUNCTION("void AdminReport(Character @, string reportee, string message)", AdminReport);
		SCRIPT_REGISTER_FUNCTION("void AdminRequest(Character @, string message)", AdminRequest);
		//SCRIPT_REGISTER_FUNCTION_PR("void Reboot()", Reboot, (), void);
		//SCRIPT_REGISTER_FUNCTION_PR("void Reboot(int, string)", Reboot, (int, std::string), void);
		SCRIPT_REGISTER_FUNCTION("void Kick(Character @, Character @, bool announce)", Kick);
		SCRIPT_REGISTER_FUNCTION("void Jail(Character @, Character @, bool announce)", Jail);
		SCRIPT_REGISTER_FUNCTION("void Ban(Character @, Character @, int, bool announce)", Ban);
		//SCRIPT_REGISTER_FUNCTION("int CheckBan(const string @, const IPAddress @, const int @)", CheckBan);
		SCRIPT_REGISTER_FUNCTION("Character @GetCharacter(string)", GetCharacter);
		SCRIPT_REGISTER_FUNCTION("Character @GetCharacterPID(uint)", GetCharacterPID);
		SCRIPT_REGISTER_FUNCTION("Character @GetCharacterCID(uint)", GetCharacterCID);
		SCRIPT_REGISTER_FUNCTION_PR("Map @GetMap(int16)", GetMap, (short), Map *);
		SCRIPT_REGISTER_FUNCTION_PR("Home @GetHome(Character @)", GetHome, (Character *), Home *);
		SCRIPT_REGISTER_FUNCTION_PR("Home @GetHome(string)", GetHome, (std::string), Home *);
		SCRIPT_REGISTER_FUNCTION("bool CharacterExists(string)", CharacterExists);
		SCRIPT_REGISTER_FUNCTION("Character @CreateCharacter(Player @, string, Gender, int, int, Skin)", CreateCharacter);
		SCRIPT_REGISTER_FUNCTION("void DeleteCharacter(string)", DeleteCharacter);
		SCRIPT_REGISTER_FUNCTION_PR("Player @Login(string, string)", Login, (std::string, std::string), Player *);
		SCRIPT_REGISTER_FUNCTION("void CreatePlayer(string, string, string, string, string, string, string, string)", CreatePlayer);
		SCRIPT_REGISTER_FUNCTION("bool PlayerExists(string)", PlayerExists);
		SCRIPT_REGISTER_FUNCTION("bool PlayerOnline(string)", PlayerOnline);
		SCRIPT_REGISTER_FUNCTION_PR("bool PKExcept(const Map @)", PKExcept, (const Map *), bool);
		SCRIPT_REGISTER_FUNCTION_PR("bool PKExcept(int)", PKExcept, (int), bool);
	SCRIPT_REGISTER_END()
};

#endif // WORLD_HPP_INCLUDED
