
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include <limits>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <cerrno>

#include "config.hpp"
#include "socket.hpp"
#include "eoserver.hpp"
#include "eoclient.hpp"
#include "packet.hpp"
#include "util.hpp"

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#include "extra/ntservice.hpp"
#endif // defined(WIN32) || defined(WIN64)

volatile bool eoserv_running = true;

static EOServer *eoserv_rehash_server = 0;

static void eoserv_rehash(int signal)
{
	if (eoserv_rehash_server == 0) return;

	std::puts("Reloading config");
	try
	{
		eoserv_rehash_server->world->config.Read("config.ini");
		eoserv_rehash_server->world->admin_config.Read("admin.ini");
	}
	catch (std::runtime_error)
	{

	}
}

template <typename T> static void eoserv_config_default(Config &config, const char *key, T value)
{
	if (config.find(key) == config.end())
	{
		config[key] = util::variant(value);
		std::fprintf(stderr, "WARNING: Could not load config value '%s' - using default (%s)\n", key, static_cast<std::string>(config[key]).c_str());
	}
}

int main(int argc, char *argv[])
{
	// Type checks
	if (std::numeric_limits<unsigned char>::digits < 8){ std::fputs("You cannot run this program (uchar is less than 8 bits)\n", stderr); std::exit(1); }
	if (std::numeric_limits<unsigned short>::digits < 16){ std::fputs("You cannot run this program (ushort is less than 16 bits)\n", stderr); std::exit(1); }
	if (std::numeric_limits<unsigned int>::digits < 32){ std::fputs("You cannot run this program (uint is less than 32 bits)\n", stderr); std::exit(1); }

	if (std::numeric_limits<char>::digits < 7){ std::fputs("You cannot run this program (char is less than 8 bits)\n", stderr); std::exit(1); }
	if (std::numeric_limits<short>::digits < 15){ std::fputs("You cannot run this program (short is less than 16 bits)\n", stderr); std::exit(1); }
	if (std::numeric_limits<int>::digits < 31){ std::fputs("You cannot run this program (int is less than 32 bits)\n", stderr); std::exit(1); }

	if (!std::numeric_limits<char>::is_signed) std::fputs("WARNING: char is not signed, correct operation of the server cannot be guaranteed.\n", stderr);

#if defined(WIN32) || defined(WIN64)
	if (argc >= 2)
	{
		std::string mode(argv[1]);
		std::string name = "EOSERV";
		bool silent = false;

		if (argc >= 3)
		{
			name = argv[2];
		}

		if (argc >= 4)
		{
			if (std::string(argv[3]) == "silent")
			{
				silent = true;
			}
		}

		if (mode == "service")
		{
			char cwd[MAX_PATH];
			GetModuleFileName(0, cwd, MAX_PATH);

			char *lastslash = 0;

			for (char *p = cwd; *p != '\0'; ++p)
			{
				if (*p == '\\' || *p == '/')
				{
					lastslash = p;
				}
			}

			if (lastslash)
			{
				*(lastslash+1) = '\0';
			}

			SetCurrentDirectory(cwd);
			service_init(name.c_str());
		}
		else if (mode == "install")
		{
			if (service_install(name.c_str()))
			{
				if (!silent) MessageBox(0, "Service installed.", "EOSERV", MB_OK);
				return 0;
			}
			else
			{
				if (!silent) MessageBox(0, OSErrorString(), "EOSERV", MB_OK);
				return 1;
			}
		}
		else if (mode == "uninstall")
		{
			if (service_uninstall(name.c_str()))
			{
				if (!silent) MessageBox(0, "Service uninstalled.", "EOSERV", MB_OK);
				return 0;
			}
			else
			{
				if (!silent) MessageBox(0, OSErrorString(), "EOSERV", MB_OK);
				return 1;
			}
		}

		return 0;
	}
#endif // defined(WIN32) || defined(WIN64)

#ifdef SIGHUP
	signal(SIGHUP, eoserv_rehash);
#endif // SIGHUP

	try
	{
		Config config, aconfig;

		try
		{
			config.Read("config.ini");
		}
		catch (std::runtime_error)
		{
			std::fputs("WARNING: Could not load config.ini - using defaults\n", stderr);
		}

		try
		{
			aconfig.Read("admin.ini");
		}
		catch (std::runtime_error)
		{
			std::fputs("WARNING: Could not load admin.ini - using defaults\n", stderr);
		}

		eoserv_config_default(config, "LogOut"             , "-");
		eoserv_config_default(config, "LogErr"             , "error.log");
		eoserv_config_default(config, "Host"               , "0.0.0.0");
		eoserv_config_default(config, "Port"               , 8078);
		eoserv_config_default(config, "MaxConnections"     , 300);
		eoserv_config_default(config, "ListenBacklog"      , 50);
		eoserv_config_default(config, "MaxPlayers"         , 200);
		eoserv_config_default(config, "MaxConnectionsPerIP", 3);
		eoserv_config_default(config, "MaxConnectionsPerPC", 1);
		eoserv_config_default(config, "IPReconnectLimit"   , 10);
		eoserv_config_default(config, "PasswordSalt"       , "ChangeMe");
		eoserv_config_default(config, "DBType"             , "mysql");
		eoserv_config_default(config, "DBHost"             , "localhost");
		eoserv_config_default(config, "DBUser"             , "eoserv");
		eoserv_config_default(config, "DBPass"             , "eoserv");
		eoserv_config_default(config, "DBName"             , "eoserv");
		eoserv_config_default(config, "EIF"                , "./data/pub/dat001.eif");
		eoserv_config_default(config, "ENF"                , "./data/pub/dtn001.enf");
		eoserv_config_default(config, "ESF"                , "./data/pub/dsl001.esf");
		eoserv_config_default(config, "ECF"                , "./data/pub/dat001.ecf");
		eoserv_config_default(config, "NewsFile"           , "./data/news.txt");
		eoserv_config_default(config, "DropsFile"          , "./data/drops.ini");
		eoserv_config_default(config, "ShopsFile"          , "./data/shops.ini");
		eoserv_config_default(config, "MapDir"             , "./data/maps/");
		eoserv_config_default(config, "Maps"               , 278);
		eoserv_config_default(config, "QuestDir"           , "./data/quests/");
		eoserv_config_default(config, "ScriptDir"          , "./data/scripts/");
		eoserv_config_default(config, "SLN"                , 1);
		eoserv_config_default(config, "SLNURL"             , "http://eoserv.net/SLN/");
		eoserv_config_default(config, "SLNSite"            , "");
		eoserv_config_default(config, "ServerName"         , "Untitled Server");
		eoserv_config_default(config, "SLNPeriod"          , 600);
		eoserv_config_default(config, "SLNZone"            , "");
		eoserv_config_default(config, "SLNBind"            , "1");
		eoserv_config_default(config, "GuildPrice"         , 50000);
		eoserv_config_default(config, "RecruitCost"        , 1000);
		eoserv_config_default(config, "GuildMaxMembers"    , 5000);
		eoserv_config_default(config, "GuildBankMax"       , 10000000);
		eoserv_config_default(config, "NPCChaseMode"       , 0);
		eoserv_config_default(config, "NPCChaseDistance"   , 18);
		eoserv_config_default(config, "NPCBoredTimer"      , 30);
		eoserv_config_default(config, "NPCAdjustMaxDam"    , 3);
		eoserv_config_default(config, "ShowLevel"          , 0);
		eoserv_config_default(config, "PKServer"           , 0);
		eoserv_config_default(config, "PKRestrict"         , 5);
		eoserv_config_default(config, "WarpBubbles"        , 1);
		eoserv_config_default(config, "HideGlobal"         , 0);
		eoserv_config_default(config, "GlobalBuffer"       , 0);
		eoserv_config_default(config, "AdminPrefix"        , "$");
		eoserv_config_default(config, "StatPerLevel"       , 3);
		eoserv_config_default(config, "SkillPerLevel"      , 3);
		eoserv_config_default(config, "EnforceWeight"      , 2);
		eoserv_config_default(config, "MaxWeight"          , 250);
		eoserv_config_default(config, "MaxLevel"           , 250);
		eoserv_config_default(config, "MaxExp"             , 2000000000);
		eoserv_config_default(config, "MaxStat"            , 1000);
		eoserv_config_default(config, "MaxSkillLevel"      , 100);
		eoserv_config_default(config, "MaxSkills"          , 48);
		eoserv_config_default(config, "MaxMessageLength"   , 128);
		eoserv_config_default(config, "MaxCharacters"      , 3);
		eoserv_config_default(config, "MaxShopBuy"         , 4);
		eoserv_config_default(config, "GhostTimer"         , 4);
		eoserv_config_default(config, "AttackLimit"        , 251);
		eoserv_config_default(config, "DropTimer"          , 120);
		eoserv_config_default(config, "DropAmount"         , 15);
		eoserv_config_default(config, "ProctectPlayerDrop" , 5);
		eoserv_config_default(config, "ProtectNPCDrop"     , 30);
		eoserv_config_default(config, "SeeDistance"        , 11);
		eoserv_config_default(config, "DropDistance"       , 2);
		eoserv_config_default(config, "RangedDistance"     , 5);
		eoserv_config_default(config, "ItemDespawn"        , 0);
		eoserv_config_default(config, "ItemDespawnCheck"   , 60);
		eoserv_config_default(config, "ItemDespawnRate"    , 600);
		eoserv_config_default(config, "ChatLength"         , 128);
		eoserv_config_default(config, "ShareMode"          , 2);
		eoserv_config_default(config, "PartyShareMode"     , 2);
		eoserv_config_default(config, "GhostNPC"           , 0);
		eoserv_config_default(config, "AllowStats"         , 1);
		eoserv_config_default(config, "StartMap"           , 0);
		eoserv_config_default(config, "StartX"             , 0);
		eoserv_config_default(config, "StartY"             , 0);
		eoserv_config_default(config, "SpawnMap"           , 0);
		eoserv_config_default(config, "SpawnX"             , 0);
		eoserv_config_default(config, "SpawnY"             , 0);
		eoserv_config_default(config, "JailMap"            , 76);
		eoserv_config_default(config, "JailX"              , 6);
		eoserv_config_default(config, "JailY"              , 5);
		eoserv_config_default(config, "StartItems"         , "");
		eoserv_config_default(config, "StartSpells"        , "");
		eoserv_config_default(config, "StartEquipMale"     , "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,");
		eoserv_config_default(config, "StartEquipFemale"   , "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,");
		eoserv_config_default(config, "MaxHairStyle"       , 20);
		eoserv_config_default(config, "MaxHairColor"       , 9);
		eoserv_config_default(config, "MaxSkin"            , 6);
		eoserv_config_default(config, "CreateMinHairStyle" , 1);
		eoserv_config_default(config, "CreateMaxHairStyle" , 20);
		eoserv_config_default(config, "CreateMinHairColor" , 0);
		eoserv_config_default(config, "CreateMaxHairColor" , 9);
		eoserv_config_default(config, "CreateMinSkin"      , 0);
		eoserv_config_default(config, "CreateMaxSkin"      , 3);
		eoserv_config_default(config, "DefaultBanLength"   , "2h");
		eoserv_config_default(config, "ExpRate"            , 100);
		eoserv_config_default(config, "DropRate"           , 100);
		eoserv_config_default(config, "MobRate"            , 100);
		eoserv_config_default(config, "SpawnRate"          , 100);
		eoserv_config_default(config, "BankUpgradeBase"    , 1000);
		eoserv_config_default(config, "BankUpgradeStep"    , 1000);
		eoserv_config_default(config, "MaxBankGold"        , 2000000000);
		eoserv_config_default(config, "MaxItem"            , 10000000);
		eoserv_config_default(config, "MaxDrop"            , 10000000);
		eoserv_config_default(config, "MaxChest"           , 10000000);
		eoserv_config_default(config, "MaxBank"            , 200);
		eoserv_config_default(config, "BaseBankSize"       , 25);
		eoserv_config_default(config, "BankSizeStep"       , 5);
		eoserv_config_default(config, "MaxBankUpgrades"    , 7);
		eoserv_config_default(config, "MaxTile"            , 8);
		eoserv_config_default(config, "MaxMap"             , 400);
		eoserv_config_default(config, "MaxTrade"           , 10000000);

		eoserv_config_default(aconfig, "item"          , 1);
		eoserv_config_default(aconfig, "npc"           , 1);
		eoserv_config_default(aconfig, "spell"         , 1);
		eoserv_config_default(aconfig, "class"         , 1);
		eoserv_config_default(aconfig, "info"          , 1);
		eoserv_config_default(aconfig, "kick"          , 1);
		eoserv_config_default(aconfig, "skick"         , 3);
		eoserv_config_default(aconfig, "jail"          , 1);
		eoserv_config_default(aconfig, "sjail"         , 3);
		eoserv_config_default(aconfig, "ban"           , 2);
		eoserv_config_default(aconfig, "sban"          , 3);
		eoserv_config_default(aconfig, "warp"          , 2);
		eoserv_config_default(aconfig, "warptome"      , 2);
		eoserv_config_default(aconfig, "warpmeto"      , 2);
		eoserv_config_default(aconfig, "evacuate"      , 2);
		eoserv_config_default(aconfig, "shutdown"      , 4);
		eoserv_config_default(aconfig, "rehash"        , 4);
		eoserv_config_default(aconfig, "sitem"         , 3);
		eoserv_config_default(aconfig, "ditem"         , 3);
		eoserv_config_default(aconfig, "learn"         , 3);
		eoserv_config_default(aconfig, "quake"         , 2);
		eoserv_config_default(aconfig, "setlevel"      , 3);
		eoserv_config_default(aconfig, "setexp"        , 3);
		eoserv_config_default(aconfig, "setstr"        , 3);
		eoserv_config_default(aconfig, "setint"        , 3);
		eoserv_config_default(aconfig, "setwis"        , 3);
		eoserv_config_default(aconfig, "setagi"        , 3);
		eoserv_config_default(aconfig, "setcon"        , 3);
		eoserv_config_default(aconfig, "setcha"        , 3);
		eoserv_config_default(aconfig, "setstatpoints" , 3);
		eoserv_config_default(aconfig, "setskillpoints", 3);
		eoserv_config_default(aconfig, "settitle"      , 3);
		eoserv_config_default(aconfig, "setpartner"    , 3);
		eoserv_config_default(aconfig, "sethome"       , 3);
		eoserv_config_default(aconfig, "sethomemap"    , 3);
		eoserv_config_default(aconfig, "sethomex"      , 3);
		eoserv_config_default(aconfig, "sethomey"      , 3);
		eoserv_config_default(aconfig, "setgender"     , 3);
		eoserv_config_default(aconfig, "sethairstyle"  , 3);
		eoserv_config_default(aconfig, "sethaircolor"  , 3);
		eoserv_config_default(aconfig, "setrace"       , 3);
		eoserv_config_default(aconfig, "setguild"      , 3);
		eoserv_config_default(aconfig, "setguildrank"  , 3);
		eoserv_config_default(aconfig, "setkarma"      , 3);
		eoserv_config_default(aconfig, "strip"         , 3);
		eoserv_config_default(aconfig, "killnpc"       , 2);

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

		std::time_t rawtime;
		char timestr[256];
		std::time(&rawtime);
		std::strftime(timestr, 256, "%c", std::localtime(&rawtime));

		std::string logerr = static_cast<std::string>(config["LogErr"]);
		if (!logerr.empty() && logerr.compare("-") != 0)
		{
			std::printf("Redirecting errors to '%s'...\n", logerr.c_str());
			if (!std::freopen(logerr.c_str(), "a", stderr))
			{
				std::fputs("Failed to redirect output.\n", stderr);
			}
			else
			{
				std::fprintf(stderr, "\n\n--- %s ---\n\n", timestr);
			}

			if (!std::setvbuf(stderr, 0, _IONBF, 0) == 0)
			{
				std::fputs("Failed to change stderr buffer settings\n", stderr);
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
			else
			{
				std::printf("\n\n--- %s ---\n\n", timestr);
			}

			if (!std::setvbuf(stdout, 0, _IONBF, 0) == 0)
			{
				std::fputs("Failed to change stdout buffer settings\n", stderr);
			}
		}

		util::array<std::string, 5> dbinfo;
		dbinfo[0] = static_cast<std::string>(config["DBType"]);
		dbinfo[1] = static_cast<std::string>(config["DBHost"]);
		dbinfo[2] = static_cast<std::string>(config["DBUser"]);
		dbinfo[3] = static_cast<std::string>(config["DBPass"]);
		dbinfo[4] = static_cast<std::string>(config["DBName"]);

		EOServer server(static_cast<std::string>(config["Host"]), static_cast<int>(config["Port"]), dbinfo, config, aconfig);
		eoserv_rehash_server = &server;
		server.Listen(static_cast<int>(config["MaxConnections"]), static_cast<int>(config["ListenBacklog"]));
		std::printf("Listening on %s:%i (0/%i connections)\n", static_cast<std::string>(config["Host"]).c_str(), static_cast<int>(config["Port"]), static_cast<int>(config["MaxConnections"]));

		// This also doubles as a check for table existance :P
		try
		{
			Database_Result acc_count = server.world->db.Query("SELECT COUNT(1) AS `count` FROM `accounts`");
			Database_Result character_count = server.world->db.Query("SELECT COUNT(1) AS `count` FROM `characters`");
			Database_Result admin_character_count = server.world->db.Query("SELECT COUNT(1) AS `count` FROM `characters` WHERE `admin` > 0");
			Database_Result guild_count = server.world->db.Query("SELECT COUNT(1) AS `count` FROM `guilds`");

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

		std::vector<EOClient *> active_clients;
		Client *newclient;
		while (eoserv_running)
		{
			if ((newclient = server.Poll()) != 0)
			{
				int ip_connections = 0;
				//bool throttle = false;

				UTIL_LIST_FOREACH_ALL(server.clients, EOClient *, client)
				{
					if (client->GetRemoteAddr() == newclient->GetRemoteAddr())
					{
						++ip_connections;
					}
				}

				/*if (throttle)
				{
					std::printf("Connection from %s was rejected (reconnecting too fast)\n", static_cast<std::string>(newclient->GetRemoteAddr()).c_str());
					newclient->Close();
				}
				else */if (ip_connections > static_cast<int>(config["MaxConnectionsPerIP"]))
				{
					std::printf("Connection from %s was rejected (too many connections from this address)\n", static_cast<std::string>(newclient->GetRemoteAddr()).c_str());
					newclient->Close();
				}
				else
				{
					std::printf("New connection from %s (%i/%i connections)\n", static_cast<std::string>(newclient->GetRemoteAddr()).c_str(), server.Connections(), server.MaxConnections());
				}
			}

			try
			{
				active_clients = server.Select(0.001);
			}
			catch (Socket_SelectFailed &e)
			{
				if (errno != EINTR)
				{
					throw;
				}
			}

			UTIL_VECTOR_IFOREACH_ALL(active_clients, EOClient *, ci)
			{
				EOClient *cl = *ci;
				std::string data;
				int done = false;
				int oldlength;

				data = cl->Recv((cl->packet_state == EOClient::ReadData)?cl->length:1);

				while (data.length() > 0 && !done)
				{
					switch (cl->packet_state)
					{
						case EOClient::ReadLen1:
							cl->raw_length[0] = data[0];
							data.erase(0,1);
							cl->packet_state = EOClient::ReadLen2;
							break;

						case EOClient::ReadLen2:
							cl->raw_length[1] = data[0];
							data.erase(0,1);
							cl->length = PacketProcessor::Number(cl->raw_length[0], cl->raw_length[1]);
							cl->packet_state = EOClient::ReadData;
							break;

						case EOClient::ReadData:
							oldlength = cl->data.length();
							cl->data += data.substr(0, cl->length);
							cl->length -= cl->data.length() - oldlength;
							if (cl->length == 0)
							{
								try
								{
									cl->Execute(cl->data);
								}
								catch (Socket_Exception &e)
								{
									std::fprintf(stderr, "Client caused an exception and was closed: %s.\n", static_cast<std::string>(cl->GetRemoteAddr()).c_str());
									std::fprintf(stderr, "%s: %s\n", e.what(), e.error());
									cl->Close();
								}
								catch (Database_Exception &e)
								{
									std::fprintf(stderr, "Client caused an exception and was closed: %s.\n", static_cast<std::string>(cl->GetRemoteAddr()).c_str());
									std::fprintf(stderr, "%s: %s\n", e.what(), e.error());
									cl->Close();
								}
								catch (std::runtime_error &e)
								{
									std::fprintf(stderr, "Client caused an exception and was closed: %s.\n", static_cast<std::string>(cl->GetRemoteAddr()).c_str());
									std::fprintf(stderr, "Runtime Error: %s\n", e.what());
									cl->Close();
								}
								catch (std::logic_error &e)
								{
									std::fprintf(stderr, "Client caused an exception and was closed: %s.\n", static_cast<std::string>(cl->GetRemoteAddr()).c_str());
									std::fprintf(stderr, "Logic Error: %s\n", e.what());
									cl->Close();
								}
								catch (std::exception &e)
								{
									std::fprintf(stderr, "Client caused an exception and was closed: %s.\n", static_cast<std::string>(cl->GetRemoteAddr()).c_str());
									std::fprintf(stderr, "Uncaught Exception: %s\n", e.what());
									cl->Close();
								}
								catch (...)
								{
									std::fprintf(stderr, "Client caused an exception and was closed: %s.\n", static_cast<std::string>(cl->GetRemoteAddr()).c_str());
									cl->Close();
								}

								cl->data.erase();
								cl->packet_state = EOClient::ReadLen1;

								done = true;
							}
							break;

						default:
							// If the code ever gets here, something is broken, so we just reset the client's state.
							data.erase();
							cl->data.erase();
							cl->packet_state = EOClient::ReadLen1;
					}
				}
			}

			server.BuryTheDead();

			server.world->timer.Tick();
		}
	}
	catch (Socket_Exception &e)
	{
		std::fprintf(stderr, "%s: %s\n", e.what(), e.error());
		return 1;
	}
	catch (Database_Exception &e)
	{
		std::fprintf(stderr, "%s: %s\n", e.what(), e.error());
		return 1;
	}
	catch (std::runtime_error &e)
	{
		std::fprintf(stderr, "Runtime Error: %s\n", e.what());
		return 1;
	}
	catch (std::logic_error &e)
	{
		std::fprintf(stderr, "Logic Error: %s\n", e.what());
		return 1;
	}
	catch (std::exception &e)
	{
		std::fprintf(stderr, "Uncaught Exception: %s\n", e.what());
		return 1;
	}
	catch (...)
	{
		std::fprintf(stderr, "Uncaught Exception\n");
		return 1;
	}

	return 0;
}
