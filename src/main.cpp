/* main.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "config.hpp"
#include "database.hpp"
#include "eoserv_config.hpp"
#include "eoserver.hpp"
#include "world.hpp"

#include "console.hpp"
#include "socket.hpp"

#include <array>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <limits>
#include <stdexcept>
#include <string>

#include "platform.h"
#include "version.h"

#ifdef WIN32
#include "eoserv_windows.h"
#include "extra/ntservice.hpp"
#endif // WIN32

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

#ifndef DEBUG
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
#endif // DEBUG

#ifdef WIN32
HANDLE eoserv_close_event;

static BOOL WINAPI eoserv_win_event_handler(DWORD event)
{
	(void)event;
	eoserv_sig_abort = true;

	WaitForSingleObject(eoserv_close_event, INFINITE);

	return TRUE;
}
#endif // WIN32

static_assert(std::numeric_limits<unsigned char>::digits >= 8, "You cannot run this program (char is less than 8 bits)");
static_assert(std::numeric_limits<unsigned short>::digits >= 16, "You cannot run this program (short is less than 16 bits)");
static_assert(std::numeric_limits<unsigned int>::digits >= 32, "You cannot run this program (int is less than 32 bits)");

static void exception_test() throw()
{
	try
	{
		throw std::runtime_error("You cannot run this program. Exception handling is working incorrectly.");
	}
	catch (std::exception& e)
	{
		// Ignore
	}
}

int eoserv_main(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	eoserv_main(argc, argv);
}

int eoserv_main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	// Type checks
	if (!std::numeric_limits<char>::is_signed) Console::Wrn("char is not signed, correct operation of the server cannot be guaranteed.");

	exception_test();

#ifdef WIN32
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
#endif // WIN32

#ifdef SIGHUP
	std::signal(SIGHUP, eoserv_rehash);
#endif // SIGHUP

	std::signal(SIGABRT, eoserv_terminate);
	std::signal(SIGTERM, eoserv_terminate);
	std::signal(SIGINT, eoserv_terminate);

#ifndef DEBUG
	std::signal(SIGSEGV, eoserv_crash);
	std::signal(SIGFPE, eoserv_crash);
#ifdef SIGBUS
	std::signal(SIGBUS, eoserv_crash);
#endif // SIGBUS
	std::signal(SIGILL, eoserv_crash);
#endif

#ifdef WIN32
	eoserv_close_event = CreateEvent(NULL, FALSE, FALSE, NULL);

	SetConsoleTitle("EOSERV");

	if (!SetConsoleCtrlHandler(static_cast<PHANDLER_ROUTINE>(eoserv_win_event_handler), TRUE))
	{
		Console::Err("Could not install Windows console event handler");
		Console::Err("$shutdown must be used to exit the server cleanly");
	}
#endif // WIN32

#ifndef DEBUG_EXCEPTIONS
	try
	{
#endif // DEBUG_EXCEPTIONS
		Config config, aconfig;

		try
		{
			config.Read("config.ini");
		}
		catch (std::runtime_error &e)
		{
			Console::Wrn("Could not load config.ini - using defaults");
		}

		try
		{
			aconfig.Read("admin.ini");
		}
		catch (std::runtime_error &e)
		{
			Console::Err("Could not load admin.ini - using defaults");
		}

		eoserv_config_validate_config(config);
		eoserv_config_validate_admin(aconfig);

		Console::Styled[1] = Console::Styled[0] = config["StyleConsole"];

		std::puts("\
                          ___ ___  ___ ___ _____   __\n\
   EOSERV Version " EOSERV_VERSION_STRING "  | __/ _ \\/ __| __| _ \\ \\ / /    http://eoserv.net/\n\
=========================| _| (_) \\__ \\ _||   /\\ ` /===========================\n\
                         |___\\___/|___/___|_|_\\ \\_/ Copyright (c) Julian Smythe\n\
\n");
#ifdef DEBUG
		Console::Wrn("This is a debug build and shouldn't be used for live servers.");
#endif

		{
			std::time_t rawtime;
			char timestr[256];
			std::time(&rawtime);
			std::strftime(timestr, 256, "%c", std::localtime(&rawtime));

			auto redirect_stream = [&](const char* name, FILE* fh, Console::Stream stream_id, const char* cfgkey)
			{
				std::string log = config[cfgkey];

				if (!log.empty() && log.compare("-") != 0)
				{
					Console::Out("Redirecting %s to '%s'...", name, log.c_str());

					if (!std::freopen(log.c_str(), "a", fh))
					{
						Console::Err("Failed to redirect %s.", name);
					}
					else
					{
						Console::Styled[stream_id] = false;
						std::fprintf(fh, "\n\n--- %s ---\n\n", timestr);
					}
				}

				if (std::setvbuf(fh, 0, _IOLBF, BUFSIZ) != 0)
					Console::Wrn("Failed to change %s buffer settings", name);
			};

			redirect_stream("errors", stderr, Console::STREAM_ERR, "LogErr");
			redirect_stream("output", stdout, Console::STREAM_OUT, "LogOut");
		}

		std::array<std::string, 6> dbinfo;
		dbinfo[0] = std::string(config["DBType"]);
		dbinfo[1] = std::string(config["DBHost"]);
		dbinfo[2] = std::string(config["DBUser"]);
		dbinfo[3] = std::string(config["DBPass"]);
		dbinfo[4] = std::string(config["DBName"]);
		dbinfo[5] = std::string(config["DBPort"]);

		EOServer server(static_cast<std::string>(config["Host"]), static_cast<int>(config["Port"]), dbinfo, config, aconfig);
		server.Listen(server.MaxConnections(), int(config["ListenBacklog"]));
		Console::Out("Listening on %s:%i (0/%i connections)", std::string(config["Host"]).c_str(), int(config["Port"]), server.MaxConnections());

		bool tables_exist = false;
		bool tried_install = false;

		while (!tables_exist)
		{
			bool try_install = false;

			try
			{
				Database_Result acc_count = server.world->db.Query("SELECT COUNT(1) AS `count` FROM `accounts`");
				Database_Result character_count = server.world->db.Query("SELECT COUNT(1) AS `count` FROM `characters`");
				Database_Result admin_character_count = server.world->db.Query("SELECT COUNT(1) AS `count` FROM `characters` WHERE `admin` > 0");
				Database_Result guild_count = server.world->db.Query("SELECT COUNT(1) AS `count` FROM `guilds`");
				Database_Result ban_count = server.world->db.Query("SELECT COUNT(1) AS `count` FROM `bans`");
				Database_Result ban_active_count = server.world->db.Query("SELECT COUNT(1) AS `count` FROM `bans` WHERE `expires` <= # AND `expires` <> 0", int(std::time(0)));
				Database_Result ban_perm_count = server.world->db.Query("SELECT COUNT(1) AS `count` FROM `bans` WHERE `expires` = 0");

				Console::Out("Database info:");
				Console::Out("  Accounts:   %i", int(acc_count.front()["count"]));
				Console::Out("  Characters: %i (%i staff)", int(character_count.front()["count"]), int(admin_character_count.front()["count"]));
				Console::Out("  Guilds:     %i", int(guild_count.front()["count"]));
				Console::Out("  Bans:       %i (%i expired, %i permanent)", int(ban_count.front()["count"]), int(ban_active_count.front()["count"]), int(ban_perm_count.front()["count"]));

				server.world->UpdateAdminCount(int(admin_character_count.front()["count"]));

				tables_exist = true;
			}
			catch (Database_Exception &e)
			{
#ifdef DEBUG
				Console::Dbg("Install check: %s: %s", e.what(), e.error());
#endif // DEBUG

				if (tried_install)
				{
					Console::Err("Could not find or install tables.");
					Console::Err(e.error());
					std::exit(1);
				}

				try_install = true;
			}

			if (try_install)
			{
				tried_install = true;
				Console::Wrn("A required table is missing. Attempting to execute install.sql");

				try
				{
					server.world->CommitDB();
					server.world->db.ExecuteFile(config["InstallSQL"]);
					server.world->BeginDB();
				}
				catch (Database_Exception& e)
				{
					Console::Err("Could not install tables.");
					Console::Err(e.error());
					std::exit(1);
				}
			}
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

				std::string old_logerr = config["LogErr"];
				std::string old_logout = config["LogOut"];

				eoserv_sig_rehash = false;
				server.world->Rehash();

				// Does not support changing from file logging back to '-'
				{
					std::time_t rawtime;
					char timestr[256];
					std::time(&rawtime);
					std::strftime(timestr, 256, "%c", std::localtime(&rawtime));

					std::string logerr = config["LogErr"];
					if (!logerr.empty() && logerr.compare("-") != 0)
					{
						if (logerr != old_logerr)
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

						if (std::setvbuf(stderr, 0, _IONBF, BUFSIZ) != 0)
						{
							Console::Wrn("Failed to change stderr buffer settings");
						}
					}

					std::string logout = config["LogOut"];
					if (!logout.empty() && logout.compare("-") != 0)
					{
						if (logout != old_logout)
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

						if (std::setvbuf(stdout, 0, _IONBF, BUFSIZ) != 0)
						{
							Console::Wrn("Failed to change stdout buffer settings");
						}
					}
				}

				Console::Out("Config reloaded");
			}

			server.Tick();
		}
#ifndef DEBUG_EXCEPTIONS
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
#endif // DEBUG_EXCEPTIONS

#ifdef WIN32
	::SetEvent(eoserv_close_event);
#endif // WIN32

	return 0;
}
