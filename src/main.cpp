
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include <csignal>
#include <cerrno>
#include <limits>
#include <stdexcept>
#include <vector>

#include "character.hpp"
#include "config.hpp"
#include "console.hpp"
#include "eoclient.hpp"
#include "eoserver.hpp"
#include "packet.hpp"
#include "player.hpp"
#include "socket.hpp"
#include "world.hpp"

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#include "extra/ntservice.hpp"
#endif // defined(WIN32) || defined(WIN64)

volatile std::sig_atomic_t eoserv_sig_abort = false;
volatile std::sig_atomic_t eoserv_sig_rehash = false;
volatile bool eoserv_running = true;

#ifdef SIGHUP
static void eoserv_rehash(int signal)
{
	(void)signal;
	eoserv_sig_rehash = true;
}
#endif // SIGHUP

static void eoserv_terminate(int signal)
{
	(void)signal;
	eoserv_sig_abort = true;
}

static void eoserv_crash(int signal)
{
	const char *extype = "Unknown error";

	switch (signal)
	{
		case SIGSEGV: extype = "Segmentation fault"; break;
		case SIGFPE: extype = "Floating point exception"; break;
#ifdef SIGBUS
		case SIGBUS: extype = "Dereferenced invalid pointer"; break;
#endif // SIGBUS
		case SIGILL: extype = "Illegal instruction"; break;
	}

	Console::Err("EOSERV is dying! %s", extype);

#ifdef DEBUG
	std::signal(signal, SIG_DFL);
	std::raise(signal);
#else // DEBUG
	std::exit(1);
#endif // DEBUG
}

#if defined(WIN32) || defined(WIN64)
HANDLE eoserv_close_event;

static BOOL WINAPI eoserv_win_event_handler(DWORD event)
{
	(void)event;
	eoserv_sig_abort = true;

	WaitForSingleObject(eoserv_close_event, INFINITE);

	return TRUE;
}
#endif // defined(WIN32) || defined(WIN64)

template <typename T> static void eoserv_config_default(Config &config, const char *key, T value)
{
	if (config.find(key) == config.end())
	{
		config[key] = util::variant(value);
		Console::Wrn("Could not load config value '%s' - using default (%s)", key, static_cast<std::string>(config[key]).c_str());
	}
}

int main(int argc, char *argv[])
{
	// Type checks
	if (std::numeric_limits<unsigned char>::digits < 8){ Console::Err("You cannot run this program (uchar is less than 8 bits)"); std::exit(1); }
	if (std::numeric_limits<unsigned short>::digits < 16){ Console::Err("You cannot run this program (ushort is less than 16 bits)"); std::exit(1); }
	if (std::numeric_limits<unsigned int>::digits < 32){ Console::Err("You cannot run this program (uint is less than 32 bits)"); std::exit(1); }

	if (std::numeric_limits<char>::digits < 7){ Console::Err("You cannot run this program (char is less than 8 bits)"); std::exit(1); }
	if (std::numeric_limits<short>::digits < 15){ Console::Err("You cannot run this program (short is less than 16 bits)"); std::exit(1); }
	if (std::numeric_limits<int>::digits < 31){ Console::Err("You cannot run this program (int is less than 32 bits)"); std::exit(1); }

	if (!std::numeric_limits<char>::is_signed) Console::Wrn("char is not signed, correct operation of the server cannot be guaranteed.");

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
	std::signal(SIGHUP, eoserv_rehash);
#endif // SIGHUP

	std::signal(SIGABRT, eoserv_terminate);
	std::signal(SIGTERM, eoserv_terminate);
	std::signal(SIGINT, eoserv_terminate);

	std::signal(SIGSEGV, eoserv_crash);
	std::signal(SIGFPE, eoserv_crash);
#ifdef SIGBUS
	std::signal(SIGBUS, eoserv_crash);
#endif // SIGBUS
	std::signal(SIGILL, eoserv_crash);

#if defined(WIN32) || defined(WIN64)
	eoserv_close_event = CreateEvent(NULL, FALSE, FALSE, NULL);

	SetConsoleTitle("EOSERV");

	if (!SetConsoleCtrlHandler(static_cast<PHANDLER_ROUTINE>(eoserv_win_event_handler), TRUE))
	{
		Console::Err("Could not install Windows console event handler");
		Console::Err("$shutdown must be used to exit the server cleanly");
	}
#endif // defined(WIN32) || defined(WIN64)

	try
	{
		Config config, aconfig;

		try
		{
			config.Read("config.ini");
		}
		catch (std::runtime_error)
		{
			Console::Wrn("Could not load config.ini - using defaults");
		}

		try
		{
			aconfig.Read("admin.ini");
		}
		catch (std::runtime_error)
		{
			Console::Err("Could not load admin.ini - using defaults");
		}

		eoserv_config_default(config, "LogOut"             , "-");
		eoserv_config_default(config, "LogErr"             , "error.log");
		eoserv_config_default(config, "StyleConsole"       , true);
		eoserv_config_default(config, "Host"               , "0.0.0.0");
		eoserv_config_default(config, "Port"               , 8078);
		eoserv_config_default(config, "MaxConnections"     , 300);
		eoserv_config_default(config, "ListenBacklog"      , 50);
		eoserv_config_default(config, "MaxPlayers"         , 200);
		eoserv_config_default(config, "MaxConnectionsPerIP", 3);
		eoserv_config_default(config, "IPReconnectLimit"   , 10);
		eoserv_config_default(config, "MaxConnectionsPerPC", 1);
		eoserv_config_default(config, "CheckVersion"       , true);
		eoserv_config_default(config, "MinVersion"         , 0);
		eoserv_config_default(config, "MaxVersion"         , 0);
		eoserv_config_default(config, "OldVersionCompat"   , false);
		eoserv_config_default(config, "TimedSave"          , 0);
		eoserv_config_default(config, "PasswordSalt"       , "ChangeMe");
		eoserv_config_default(config, "DBType"             , "mysql");
		eoserv_config_default(config, "DBHost"             , "localhost");
		eoserv_config_default(config, "DBUser"             , "eoserv");
		eoserv_config_default(config, "DBPass"             , "eoserv");
		eoserv_config_default(config, "DBName"             , "eoserv");
		eoserv_config_default(config, "DBPort"             , 0);
		eoserv_config_default(config, "EIF"                , "./data/pub/dat001.eif");
		eoserv_config_default(config, "ENF"                , "./data/pub/dtn001.enf");
		eoserv_config_default(config, "ESF"                , "./data/pub/dsl001.esf");
		eoserv_config_default(config, "ECF"                , "./data/pub/dat001.ecf");
		eoserv_config_default(config, "NewsFile"           , "./data/news.txt");
		eoserv_config_default(config, "DropsFile"          , "./data/drops.ini");
		eoserv_config_default(config, "ShopsFile"          , "./data/shops.ini");
		eoserv_config_default(config, "ArenasFile"         , "./data/arenas.ini");
		eoserv_config_default(config, "FormulasFile"       , "./data/formulas.ini");
		eoserv_config_default(config, "HomeFile"           , "./data/home.ini");
		eoserv_config_default(config, "SkillsFile"         , "./data/skills.ini");
		eoserv_config_default(config, "MapDir"             , "./data/maps/");
		eoserv_config_default(config, "Maps"               , 278);
		eoserv_config_default(config, "SLN"                , true);
		eoserv_config_default(config, "SLNURL"             , "http://eoserv.net/SLN/");
		eoserv_config_default(config, "SLNSite"            , "");
		eoserv_config_default(config, "ServerName"         , "Untitled Server");
		eoserv_config_default(config, "SLNPeriod"          , 600);
		eoserv_config_default(config, "SLNZone"            , "");
		eoserv_config_default(config, "SLNBind"            , "1");
		eoserv_config_default(config, "GuildPrice"         , 50000);
		eoserv_config_default(config, "RecruitCost"        , 1000);
		eoserv_config_default(config, "GuildMaxMembers"    , 5000);
		eoserv_config_default(config, "GuildCreateMembers" , 9);
		eoserv_config_default(config, "GuildBankMax"       , 2000000000);
		eoserv_config_default(config, "GuildDefaultRanks"  , "Leader,Recruiter,,,,,,,New Member");
		eoserv_config_default(config, "GuildShowRecruiters", true);
		eoserv_config_default(config, "GuildEditRank"      , 1);
		eoserv_config_default(config, "GuildKickRank"      , 1);
		eoserv_config_default(config, "GuildPromoteRank"   , 1);
		eoserv_config_default(config, "GuildDemoteRank"    , 1);
		eoserv_config_default(config, "GuildRecruitRank"   , 2);
		eoserv_config_default(config, "GuildMultipleFounders", true);
		eoserv_config_default(config, "GuildAnnounce"      , true);
		eoserv_config_default(config, "GuildDateFormat"    , "%Y/%m/%d");
		eoserv_config_default(config, "GuildMinDeposit"    , 1000);
		eoserv_config_default(config, "GuildMaxNameLength" , 24);
		eoserv_config_default(config, "GuildMaxDescLength" , 240);
		eoserv_config_default(config, "GuildMaxRankLength" , 16);
		eoserv_config_default(config, "GuildMaxWidth"      , 180);
		eoserv_config_default(config, "GlobalPK"           , false);
		eoserv_config_default(config, "PKExcept"           , "");
		eoserv_config_default(config, "NPCChaseMode"       , 0);
		eoserv_config_default(config, "NPCChaseDistance"   , 18);
		eoserv_config_default(config, "NPCBoredTimer"      , 30);
		eoserv_config_default(config, "NPCAdjustMaxDam"    , 3);
		eoserv_config_default(config, "BoardMaxPosts"      , 20);
		eoserv_config_default(config, "BoardMaxUserPosts"  , 6);
		eoserv_config_default(config, "BoardMaxRecentPosts", 2);
		eoserv_config_default(config, "BoardRecentPostTime", 1800);
		eoserv_config_default(config, "BoardMaxSubjectLength", 32);
		eoserv_config_default(config, "BoardMaxPostLength" , 2048);
		eoserv_config_default(config, "BoardDatePosts"     , true);
		eoserv_config_default(config, "AdminBoard"         , 8);
		eoserv_config_default(config, "AdminBoardLimit"    , 100);
		eoserv_config_default(config, "ShowLevel"          , false);
		eoserv_config_default(config, "WarpBubbles"        , true);
		eoserv_config_default(config, "HideGlobal"         , false);
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
		eoserv_config_default(config, "MaxCharacters"      , 3);
		eoserv_config_default(config, "MaxShopBuy"         , 4);
		eoserv_config_default(config, "GhostTimer"         , 4);
		eoserv_config_default(config, "AttackLimit"        , 251);
		eoserv_config_default(config, "DropTimer"          , 120);
		eoserv_config_default(config, "DropAmount"         , 15);
		eoserv_config_default(config, "ProtectPlayerDrop"  , 5);
		eoserv_config_default(config, "ProtectNPCDrop"     , 30);
		eoserv_config_default(config, "ProtectPKDrop"      , 60);
		eoserv_config_default(config, "ProtectDeathDrop"   , 300);
		eoserv_config_default(config, "SeeDistance"        , 11);
		eoserv_config_default(config, "DropDistance"       , 2);
		eoserv_config_default(config, "RangedDistance"     , 5);
		eoserv_config_default(config, "ItemDespawn"        , false);
		eoserv_config_default(config, "ItemDespawnCheck"   , 60);
		eoserv_config_default(config, "ItemDespawnRate"    , 600);
		eoserv_config_default(config, "RecoverSpeed"       , 90);
		eoserv_config_default(config, "NPCRecoverSpeed"    , 105);
		eoserv_config_default(config, "HPRecoverRate"      , 0.1);
		eoserv_config_default(config, "SitHPRecoverRate"   , 0.2);
		eoserv_config_default(config, "TPRecoverRate"      , 0.1);
		eoserv_config_default(config, "SitTPRecoverRate"   , 0.2);
		eoserv_config_default(config, "NPCRecoverRate"     , 0.1);
		eoserv_config_default(config, "ChatLength"         , 128);
		eoserv_config_default(config, "ShareMode"          , 2);
		eoserv_config_default(config, "PartyShareMode"     , 2);
		eoserv_config_default(config, "GhostNPC"           , false);
		eoserv_config_default(config, "AllowStats"         , true);
		eoserv_config_default(config, "StartMap"           , 0);
		eoserv_config_default(config, "StartX"             , 0);
		eoserv_config_default(config, "StartY"             , 0);
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
		eoserv_config_default(config, "LimitDamage"        , true);
		eoserv_config_default(config, "DeathRecover"       , 0.5);
		eoserv_config_default(config, "Deadly"             , false);
		eoserv_config_default(config, "ExpRate"            , 1.0);
		eoserv_config_default(config, "DropRate"           , 1.0);
		eoserv_config_default(config, "MobRate"            , 1.0);
		eoserv_config_default(config, "PKRate"             , 0.75);
		eoserv_config_default(config, "CriticalRate"       , 0.08);
		eoserv_config_default(config, "SpawnRate"          , 1.0);
		eoserv_config_default(config, "BarberBase"         , 0);
		eoserv_config_default(config, "BarberStep"         , 200);
		eoserv_config_default(config, "BankUpgradeBase"    , 1000);
		eoserv_config_default(config, "BankUpgradeStep"    , 1000);
		eoserv_config_default(config, "JukeboxSongs"       , 20);
		eoserv_config_default(config, "JukeboxPrice"       , 25);
		eoserv_config_default(config, "JukeboxTimer"       , 90);
		eoserv_config_default(config, "RespawnBossChildren", true);
		eoserv_config_default(config, "OldReports"         , false);
		eoserv_config_default(config, "WarpSuck"           , 15);
		eoserv_config_default(config, "DoorTimer"          , 3.0);
		eoserv_config_default(config, "ChatMaxWidth"       , 1500);
		eoserv_config_default(config, "AccountMinLength"   , 4);
		eoserv_config_default(config, "AccountMaxLength"   , 16);
		eoserv_config_default(config, "PasswordMinLength"  , 6);
		eoserv_config_default(config, "PasswordMaxLength"  , 12);
		eoserv_config_default(config, "RealNameMaxLength"  , 64);
		eoserv_config_default(config, "LocationMaxLength"  , 64);
		eoserv_config_default(config, "EmailMaxLength"     , 64);
		eoserv_config_default(config, "ComputerNameLength" , 64);
		eoserv_config_default(config, "LimitAttack"        , 251);
		eoserv_config_default(config, "MaxBankGold"        , 2000000000);
		eoserv_config_default(config, "MaxItem"            , 2000000000);
		eoserv_config_default(config, "MaxDrop"            , 10000000);
		eoserv_config_default(config, "MaxChest"           , 10000000);
		eoserv_config_default(config, "ChestSlots"         , 5);
		eoserv_config_default(config, "MaxBank"            , 200);
		eoserv_config_default(config, "BaseBankSize"       , 25);
		eoserv_config_default(config, "BankSizeStep"       , 5);
		eoserv_config_default(config, "MaxBankUpgrades"    , 7);
		eoserv_config_default(config, "MaxTile"            , 8);
		eoserv_config_default(config, "MaxMap"             , 400);
		eoserv_config_default(config, "MaxTrade"           , 2000000000);

		eoserv_config_default(aconfig, "item"          , 1);
		eoserv_config_default(aconfig, "npc"           , 1);
		eoserv_config_default(aconfig, "spell"         , 1);
		eoserv_config_default(aconfig, "class"         , 1);
		eoserv_config_default(aconfig, "info"          , 1);
		eoserv_config_default(aconfig, "uptime"        , 1);
		eoserv_config_default(aconfig, "kick"          , 1);
		eoserv_config_default(aconfig, "skick"         , 3);
		eoserv_config_default(aconfig, "jail"          , 1);
		eoserv_config_default(aconfig, "sjail"         , 3);
		eoserv_config_default(aconfig, "ban"           , 2);
		eoserv_config_default(aconfig, "sban"          , 3);
		eoserv_config_default(aconfig, "warp"          , 2);
		eoserv_config_default(aconfig, "warptome"      , 2);
		eoserv_config_default(aconfig, "warpmeto"      , 2);
		eoserv_config_default(aconfig, "hide"          , 2);
		eoserv_config_default(aconfig, "evacuate"      , 2);
		eoserv_config_default(aconfig, "remap"         , 4);
		eoserv_config_default(aconfig, "arena"         , 1);
		eoserv_config_default(aconfig, "board"         , 1);
		eoserv_config_default(aconfig, "shutdown"      , 4);
		eoserv_config_default(aconfig, "rehash"        , 4);
		eoserv_config_default(aconfig, "repub"         , 4);
		eoserv_config_default(aconfig, "sitem"         , 3);
		eoserv_config_default(aconfig, "ditem"         , 3);
		eoserv_config_default(aconfig, "snpc"          , 3);
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
		eoserv_config_default(aconfig, "setfiance"     , 3);
		eoserv_config_default(aconfig, "setpartner"    , 3);
		eoserv_config_default(aconfig, "sethome"       , 3);
		eoserv_config_default(aconfig, "setgender"     , 3);
		eoserv_config_default(aconfig, "sethairstyle"  , 3);
		eoserv_config_default(aconfig, "sethaircolor"  , 3);
		eoserv_config_default(aconfig, "setrace"       , 3);
		eoserv_config_default(aconfig, "setguild"      , 3);
		eoserv_config_default(aconfig, "setguildrank"  , 3);
		eoserv_config_default(aconfig, "setkarma"      , 3);
		eoserv_config_default(aconfig, "strip"         , 3);
		eoserv_config_default(aconfig, "killnpc"       , 2);
		eoserv_config_default(aconfig, "boardmod"      , 1);
		eoserv_config_default(aconfig, "reports"       , 1);
		eoserv_config_default(aconfig, "seehide"       , 4);

		Console::Styled[1] = Console::Styled[0] = config["StyleConsole"];

		std::puts("\
                          ___ ___  ___ ___ _____   __\n\
   EOSERV Version 0.5.2  | __/ _ \\/ __| __| _ \\ \\ / /    http://eoserv.net/\n\
=========================| _| (_) \\__ \\ _||   /\\ ` /===========================\n\
                         |___\\___/|___/___|_|_\\ \\_/    sausage@tehsausage.com\n\
\n");
#ifdef DEBUG
		Console::Wrn("This is a debug build and shouldn't be used for live servers.");
#endif

		std::time_t rawtime;
		char timestr[256];
		std::time(&rawtime);
		std::strftime(timestr, 256, "%c", std::localtime(&rawtime));

		std::string logerr = static_cast<std::string>(config["LogErr"]);
		if (!logerr.empty() && logerr.compare("-") != 0)
		{
			Console::Out("Redirecting errors to '%s'...", logerr.c_str());
			if (!std::freopen(logerr.c_str(), "a", stderr))
			{
				Console::Err("Failed to redirect errors.");
			}
			else
			{
				Console::Styled[Console::STREAM_ERR] = false;
				std::fprintf(stderr, "\n\n--- %s ---\n\n", timestr);
			}

			if (!std::setvbuf(stderr, 0, _IONBF, 0) == 0)
			{
				Console::Wrn("Failed to change stderr buffer settings");
			}
		}

		std::string logout = static_cast<std::string>(config["LogOut"]);
		if (!logout.empty() && logout.compare("-") != 0)
		{
			Console::Out("Redirecting output to '%s'...", logout.c_str());
			if (!std::freopen(logout.c_str(), "a", stdout))
			{
				Console::Err("Failed to redirect output.");
			}
			else
			{
				Console::Styled[Console::STREAM_OUT] = false;
				std::printf("\n\n--- %s ---\n\n", timestr);
			}

			if (!std::setvbuf(stdout, 0, _IONBF, 0) == 0)
			{
				Console::Wrn("Failed to change stdout buffer settings");
			}
		}

		std::array<std::string, 6> dbinfo;
		dbinfo[0] = static_cast<std::string>(config["DBType"]);
		dbinfo[1] = static_cast<std::string>(config["DBHost"]);
		dbinfo[2] = static_cast<std::string>(config["DBUser"]);
		dbinfo[3] = static_cast<std::string>(config["DBPass"]);
		dbinfo[4] = static_cast<std::string>(config["DBName"]);
		dbinfo[5] = static_cast<std::string>(config["DBPort"]);

		EOServer server(static_cast<std::string>(config["Host"]), static_cast<int>(config["Port"]), dbinfo, config, aconfig);
		server.Listen(static_cast<int>(config["MaxConnections"]), static_cast<int>(config["ListenBacklog"]));
		Console::Out("Listening on %s:%i (0/%i connections)", static_cast<std::string>(config["Host"]).c_str(), static_cast<int>(config["Port"]), static_cast<int>(config["MaxConnections"]));

		// This also doubles as a check for table existance :P
		try
		{
			Database_Result acc_count = server.world->db.Query("SELECT COUNT(1) AS `count` FROM `accounts`");
			Database_Result character_count = server.world->db.Query("SELECT COUNT(1) AS `count` FROM `characters`");
			Database_Result admin_character_count = server.world->db.Query("SELECT COUNT(1) AS `count` FROM `characters` WHERE `admin` > 0");
			Database_Result guild_count = server.world->db.Query("SELECT COUNT(1) AS `count` FROM `guilds`");

			Console::Out("Database info:");
			Console::Out("  Accounts:   %i", static_cast<int>(acc_count.front()["count"]));
			Console::Out("  Characters: %i (%i staff)", static_cast<int>(character_count.front()["count"]), static_cast<int>(admin_character_count.front()["count"]));
			Console::Out("  Guilds:     %i", static_cast<int>(guild_count.front()["count"]));
		}
		catch (Database_Exception &e)
		{
			Console::Err("A required table is missing. (Have you executed install.sql?)");
			Console::Err(e.error());
			std::exit(1);
		}

		while (eoserv_running)
		{
			if (eoserv_sig_abort)
			{
				Console::Out("Exiting EOSERV");
				eoserv_sig_abort = false;
				break;
			}

			if (eoserv_sig_rehash)
			{
				Console::Out("Reloading config");
				eoserv_sig_rehash = false;
				server.world->Rehash();
			}

			server.Tick();
		}
	}
	catch (Socket_Exception &e)
	{
		Console::Err("%s: %s", e.what(), e.error());
		return 1;
	}
	catch (Database_Exception &e)
	{
		Console::Err("%s: %s", e.what(), e.error());
		return 1;
	}
	catch (std::runtime_error &e)
	{
		Console::Err("Runtime Error: %s", e.what());
		return 1;
	}
	catch (std::logic_error &e)
	{
		Console::Err("Logic Error: %s", e.what());
		return 1;
	}
	catch (std::exception &e)
	{
		Console::Err("Uncaught Exception: %s", e.what());
		return 1;
	}
	catch (...)
	{
		Console::Err("Uncaught Exception");
		return 1;
	}

#if defined(WIN32) || defined(WIN64)
	::SetEvent(eoserv_close_event);
#endif // defined(WIN32) || defined(WIN64)

	return 0;
}
