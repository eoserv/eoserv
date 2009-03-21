
#include <limits>
#include <cstdio>
#include <cstdlib>

#include "config.hpp"
#include "socket.hpp"
#include "eoclient.hpp"
#include "packet.hpp"
#include "util.hpp"

extern Database eoserv_db;

void syspause(){ std::puts("Server terminated.\nPress enter to continue . . ."); std::getc(stdin); }

int main()
{
#ifdef DEBUG
#ifdef __GNUC__
	std::set_terminate(__gnu_cxx::__verbose_terminate_handler);
#endif
	std::atexit(syspause);
#endif

	try
	{
		bool running = true;
		Config config;
		try
		{
			Config load_config("config.ini");
			config = load_config;
		}
		catch (std::runtime_error)
		{
			std::fputs("WARNING: Could not load config.ini - using defaults\n", stderr);
		}

#define CONFIG_DEFAULT(key, value)\
if (config.find(key) == config.end())\
{\
	util::variant vv = util::variant(value);\
	config[key] = vv;\
	std::fprintf(stderr, "WARNING: Could not load config value '%s' - using default (%s)\n", key, static_cast<std::string>(vv).c_str());\
}
		CONFIG_DEFAULT("LogOut"             , "-");
		CONFIG_DEFAULT("LogErr"             , "error.log");
		CONFIG_DEFAULT("Host"               , "0.0.0.0");
		CONFIG_DEFAULT("Port"               , 8078);
		CONFIG_DEFAULT("MaxConnections"     , 300);
		CONFIG_DEFAULT("ListenBacklog"      , 50);
		CONFIG_DEFAULT("MaxPlayers"         , 200);
		CONFIG_DEFAULT("MaxConnectionsPerIP", 2);
		CONFIG_DEFAULT("PasswordSalt"       , "ChangeMe");
		CONFIG_DEFAULT("DBType"             , "mysql");
		CONFIG_DEFAULT("DBHost"             , "localhost");
		CONFIG_DEFAULT("DBUser"             , "eoserv");
		CONFIG_DEFAULT("DBPass"             , "eoserv");
		CONFIG_DEFAULT("DBName"             , "eoserv");
		CONFIG_DEFAULT("EIF"                , "./data/pub/dat001.eif");
		CONFIG_DEFAULT("ENF"                , "./data/pub/dtn001.enf");
		CONFIG_DEFAULT("ESF"                , "./data/pub/dsl001.esf");
		CONFIG_DEFAULT("ECF"                , "./data/pub/dat001.ecf");
		CONFIG_DEFAULT("NewsFile"           , "./data/news.txt");
		CONFIG_DEFAULT("MapDir"             , "./data/maps/");
		CONFIG_DEFAULT("Maps"               , 278);
		CONFIG_DEFAULT("NPCDir"             , "./data/npc/");
		CONFIG_DEFAULT("QuestDir"           , "./data/quests/");
		CONFIG_DEFAULT("ScriptDir"          , "./data/scripts/");
		CONFIG_DEFAULT("SLN"                , 1);
		CONFIG_DEFAULT("SLNURL"             , "http://eoserv.net/SLN/");
		CONFIG_DEFAULT("ServerName"         , "Untitled Server");
		CONFIG_DEFAULT("SLNPeriod"          , 900);
		CONFIG_DEFAULT("GuildPrice"         , 50000);
		CONFIG_DEFAULT("RecruitCost"        , 1000);
		CONFIG_DEFAULT("GuildMaxMembers"    , 5000);
		CONFIG_DEFAULT("GuildBankMax"       , 10000000);
		CONFIG_DEFAULT("ShowLevel"          , 0);
		CONFIG_DEFAULT("IPConnectLimit"     , 3);
		CONFIG_DEFAULT("IPReconnectLimit"   , 10);
		CONFIG_DEFAULT("PKServer"           , 0);
		CONFIG_DEFAULT("PKRestrict"         , 5);
		CONFIG_DEFAULT("WarpBubbles"        , 1);
		CONFIG_DEFAULT("HideGlobal"         , 0);
		CONFIG_DEFAULT("HideBuffer"         , 0);
		CONFIG_DEFAULT("AdminPrefix"        , "$");
		CONFIG_DEFAULT("StatPerLevel"       , 3);
		CONFIG_DEFAULT("SkillPerLevel"      , 3);
		CONFIG_DEFAULT("EnforceWeight"      , 2);
		CONFIG_DEFAULT("MaxWeight"          , 250);
		CONFIG_DEFAULT("MaxLevel"           , 250);
		CONFIG_DEFAULT("MaxExp"             , 2000000000);
		CONFIG_DEFAULT("MaxStat"            , 1000);
		CONFIG_DEFAULT("MaxSkillLevel"      , 100);
		CONFIG_DEFAULT("MaxSkills"          , 48);
		CONFIG_DEFAULT("MaxMessageLength"   , 128);
		CONFIG_DEFAULT("MaxCharacters"      , 3);
		CONFIG_DEFAULT("GhostTimer"         , 4);
		CONFIG_DEFAULT("AttackLimit"        , 251);
		CONFIG_DEFAULT("DropTimer"          , 120);
		CONFIG_DEFAULT("DropAmount"         , 15);
		CONFIG_DEFAULT("ProctectPlayerDrop" , 5);
		CONFIG_DEFAULT("ProtectNPCDrop"     , 30);
		CONFIG_DEFAULT("SeeDistance"        , 11);
		CONFIG_DEFAULT("DropDistance"       , 2);
		CONFIG_DEFAULT("ChatLength"         , 128);
		CONFIG_DEFAULT("ShareMode"          , 2);
		CONFIG_DEFAULT("PartyShareMode"     , 3);
		CONFIG_DEFAULT("GhostNPC"           , 0);
		CONFIG_DEFAULT("AllowStats"         , 1);
		CONFIG_DEFAULT("StartMap"           , 192);
		CONFIG_DEFAULT("StartX"             , 6);
		CONFIG_DEFAULT("StartY"             , 6);
		CONFIG_DEFAULT("SpawnMap"           , 192);
		CONFIG_DEFAULT("SpawnX"             , 6);
		CONFIG_DEFAULT("SpawnY"             , 6);
		CONFIG_DEFAULT("JailMap"            , 76);
		CONFIG_DEFAULT("JailX"              , 6);
		CONFIG_DEFAULT("JailY"              , 5);
		CONFIG_DEFAULT("StartItems"         , "");
		CONFIG_DEFAULT("StartSpells"        , "");
		CONFIG_DEFAULT("StartEquipMale"     , "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
		CONFIG_DEFAULT("StartEquipFemale"   , "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
		CONFIG_DEFAULT("MinHairStyle"       , 20);
		CONFIG_DEFAULT("MaxHairColor"       , 12);
		CONFIG_DEFAULT("MaxSkin"            , 6);
		CONFIG_DEFAULT("CreateMinHairStyle" , 1);
		CONFIG_DEFAULT("CreateMaxHairStyle" , 20);
		CONFIG_DEFAULT("CreateMinHairColor" , 0);
		CONFIG_DEFAULT("CreateMaxHairColor" , 12);
		CONFIG_DEFAULT("CreateMinSkin"      , 0);
		CONFIG_DEFAULT("CreateMaxSkin"      , 3);
		CONFIG_DEFAULT("DefaultBanLength"   , "2h");
		CONFIG_DEFAULT("ExpRate"            , 100);
		CONFIG_DEFAULT("DropRate"           , 100);
		CONFIG_DEFAULT("GoldRate"           , 100);
		CONFIG_DEFAULT("MobRate"            , 100);
		CONFIG_DEFAULT("MaxBankGold"        , 2000000000);
		CONFIG_DEFAULT("MaxItem"            , 10000000);
		CONFIG_DEFAULT("MaxDrop"            , 10000000);
		CONFIG_DEFAULT("MaxChest"           , 10000000);
		CONFIG_DEFAULT("MaxBank"            , 200);
		CONFIG_DEFAULT("MaxTile"            , 8);
		CONFIG_DEFAULT("MaxMap"             , 400);
		CONFIG_DEFAULT("MaxTrade"           , 10000000);
#undef CONFIG_DEFAULT

		// Type checks
		if (std::numeric_limits<unsigned char>::digits < 8){ std::fputs("You cannot run this program (uchar is less than 8 bits)\n", stderr); std::exit(1); }
		if (std::numeric_limits<unsigned short>::digits < 16){ std::fputs("You cannot run this program (ushort is less than 16 bits)\n", stderr); std::exit(1); }
		if (std::numeric_limits<unsigned int>::digits < 32){ std::fputs("You cannot run this program (uint is less than 32 bits)\n", stderr); std::exit(1); }

		if (std::numeric_limits<char>::digits < 7){ std::fputs("You cannot run this program (char is less than 8 bits)\n", stderr); std::exit(1); }
		if (std::numeric_limits<short>::digits < 15){ std::fputs("You cannot run this program (short is less than 16 bits)\n", stderr); std::exit(1); }
		if (std::numeric_limits<int>::digits < 31){ std::fputs("You cannot run this program (int is less than 32 bits)\n", stderr); std::exit(1); }

		if (std::numeric_limits<unsigned char>::digits != 8) std::fputs("WARNING: uchar is over 8 bytes, correct operation of the server cannot be guaranteed.\n", stderr);
		if (std::numeric_limits<unsigned short>::digits != 16) std::fputs("WARNING: ushort is over 16 bytes, correct operation of the server cannot be guaranteed.\n", stderr);
		if (std::numeric_limits<unsigned int>::digits != 32) std::fputs("WARNING: uint is over 32 bytes, correct operation of the server cannot be guaranteed.\n", stderr);

		if (std::numeric_limits<char>::digits > 8) std::fputs("WARNING: char is over 8 bytes, correct operation of the server cannot be guaranteed.\n", stderr);
		if (std::numeric_limits<short>::digits > 15) std::fputs("WARNING: short is over 16 bytes, correct operation of the server cannot be guaranteed.\n", stderr);
		if (std::numeric_limits<int>::digits > 31) std::fputs("WARNING: int is over 32 bytes, correct operation of the server cannot be guaranteed.\n", stderr);

		if (!std::numeric_limits<char>::is_signed) std::fputs("WARNING: char is not signed, correct operation of the server cannot be guaranteed.\n", stderr);
		if (!std::numeric_limits<short>::is_signed) std::fputs("WARNING: short is not signed, correct operation of the server cannot be guaranteed.\n", stderr);
		if (!std::numeric_limits<int>::is_signed) std::fputs("WARNING: int is not signed, correct operation of the server cannot be guaranteed.\n", stderr);

		std::puts("\
                          ___ ___  ___ ___ _____   __\n\
   EOSERV Version 0.4.0  | __/ _ \\/ __| __| _ \\ \\ / /    http://eoserv.net/\n\
=========================| _| (_) \\__ \\ _||   /\\ ` /===========================\n\
                         |___\\___/|___/___|_|_\\ \\_/    sausage@tehsausage.com\n\
\n\
EO Version Support: .27 .28\n\
\n");
#ifdef DEBUG
		std::puts("WARNING: This is a debug build and shouldn't be used for live servers.");
#endif

		std::string logerr = static_cast<std::string>(config["LogErr"]);
		if (!logerr.empty() && logerr.compare("-") != 0)
		{
			std::printf("Redirecting errors to '%s'...\n", logerr.c_str());
			if (!std::freopen(logerr.c_str(), "a", stderr))
			{
				std::fputs("Failed to redirect output.\n", stderr);
			}
		}

		std::string logout = static_cast<std::string>(config["LogOut"]);
		if (!logout.empty() && logout.compare("-") != 0)
		{
			std::printf("Redirecting output to '%s'...\n", logout.c_str());
			if (!std::freopen(logout.c_str(), "a", stdout))
			{
				std::fputs("Failed to redirect output.\n", stderr);
			}
		}

		util::array<std::string, 5> dbinfo;
		dbinfo[0] = static_cast<std::string>(config["DBType"]);
		dbinfo[1] = static_cast<std::string>(config["DBHost"]);
		dbinfo[2] = static_cast<std::string>(config["DBUser"]);
		dbinfo[3] = static_cast<std::string>(config["DBPass"]);
		dbinfo[4] = static_cast<std::string>(config["DBName"]);

		EOServer server(static_cast<std::string>(config["Host"]), static_cast<int>(config["Port"]), dbinfo, config);

		if (server.State() == Server<EOClient>::Invalid)
		{
			std::fprintf(stderr, "There was a problem initializing the server. (Is port %i already in use?)\n", static_cast<int>(config["Port"]));
			std::exit(1);
		}

		std::printf("Listening on %s:%i (0/%i connections)\n", static_cast<std::string>(config["Host"]).c_str(), static_cast<int>(config["Port"]), static_cast<int>(config["MaxConnections"]));

		if (!server.Listen(static_cast<int>(config["MaxConnections"]), static_cast<int>(config["ListenBacklog"])))
		{
			std::fprintf(stderr, "Failed to bind to address. (Is port %i already in use?)\n", static_cast<int>(config["Port"]));
			std::exit(1);
		}

		// This also doubles as a check for table existance :P
		try
		{
			Database_Result acc_count = eoserv_db.Query("SELECT COUNT(1) AS `count` FROM `accounts`");
			Database_Result character_count = eoserv_db.Query("SELECT COUNT(1) AS `count` FROM `characters`");
			Database_Result admin_character_count = eoserv_db.Query("SELECT COUNT(1) AS `count` FROM `characters` WHERE `admin` > 0");
			Database_Result guild_count = eoserv_db.Query("SELECT COUNT(1) AS `count` FROM `guilds`");

			printf("\nDatabase info:\n\
  Accounts:   %i\n\
  Characters: %i (%i staff)\n\
  Guilds:     %i\n\
\n", static_cast<int>(acc_count.front()["count"]), static_cast<int>(character_count.front()["count"]), static_cast<int>(admin_character_count.front()["count"]), static_cast<int>(guild_count.front()["count"]));
		}
		catch (Database_Exception &e)
		{
			std::fputs("A required table is missing. (Have you executed install.sql?)\n", stderr);
			std::fputs(e.error(), stderr);
			std::exit(1);
		}

		while (running)
		{
			Client *newclient;
			if ((newclient = server.Poll()) != 0)
			{
				std::printf("New connection from %s (%i/%i connections)\n", static_cast<std::string>(newclient->GetRemoteAddr()).c_str(), server.Connections(), server.MaxConnections());
			}

			std::list<EOClient *> active_clients = server.Select(10);

			for (std::list<EOClient *>::iterator ci = active_clients.begin(); ci != active_clients.end(); ++ci)
			{
				EOClient *cl = *ci;
				std::string data;
				int done = false;
				int oldlength;

				data = cl->Recv((cl->state == EOClient::ReadData)?cl->length:1);

				while (data.length() > 0 && !done)
				{
					switch (cl->state)
					{
						case EOClient::ReadLen1:
							cl->raw_length[0] = data[0];
							data.erase(0,1);
							cl->state = EOClient::ReadLen2;
							break;

						case EOClient::ReadLen2:
							cl->raw_length[1] = data[0];
							data.erase(0,1);
							cl->length = PacketProcessor::Number(cl->raw_length[0], cl->raw_length[1]);
							cl->state = EOClient::ReadData;
							break;

						case EOClient::ReadData:
							oldlength = cl->data.length();
							cl->data += data.substr(0, cl->length);
							cl->length -= cl->data.length() - oldlength;
							if (cl->length == 0)
							{
								cl->Execute(cl->data);

								cl->data.erase();
								cl->state = EOClient::ReadLen1;

								done = true;
							}
							break;

						default:
							// If the code ever gets here, something is broken, so we just reset the client's state.
							data.erase();
							cl->data.erase();
							cl->state = EOClient::ReadLen1;
					}
				}
			}

			server.BuryTheDead();

			server.world->timer.Tick();
		}
	}
	catch (Database_Exception &e)
	{
		std::fprintf(stderr,  "Database Error: %s\n", e.error());
	}
	catch (std::runtime_error &e)
	{
		std::fprintf(stderr,  "Error: %s\n", e.what());
	}

	return 0;
}
