
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
	std::atexit(syspause);
#endif

	try
	{
		bool running = true;
		Config config("config.ini");

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
   EOSERV Version 0.3.0  | __/ _ \\/ __| __| _ \\ \\ / /    http://eoserv.net/\n\
=========================| _| (_) \\__ \\ _||   /\\ ` /===========================\n\
       09 Mar 2009       |___\\___/|___/___|_|_\\ \\_/    sausage@tehsausage.com\n\
\n\
EO Version Support: .27 .28\n\
\n");
#ifdef DEBUG

#ifdef __GNUC__
		std::set_terminate(__gnu_cxx::__verbose_terminate_handler);
#endif
		std::puts("WARNING: This is a debug build and shouldn't be used for live servers.");
#endif

		std::string logerr = static_cast<std::string>(config["LogErr"]);
		if (!logerr.empty() && logerr.compare("-") != 0)
		{
			std::printf("Redirecting errors to '%s'...\n", logerr.c_str());
			if (!std::freopen(logerr.c_str(), "a", stderr))
			{
				std::fprintf(stderr, "Failed to redirect output.\n");
			}
		}

		std::string logout = static_cast<std::string>(config["LogOut"]);
		if (!logout.empty() && logout.compare("-") != 0)
		{
			std::printf("Redirecting output to '%s'...\n", logout.c_str());
			if (!std::freopen(logout.c_str(), "a", stdout))
			{
				std::fprintf(stderr, "Failed to redirect output.\n");
			}
		}

		util::array<std::string, 5> dbinfo;
		dbinfo[0] = static_cast<std::string>(config["DBType"]);
		dbinfo[1] = static_cast<std::string>(config["DBHost"]);
		dbinfo[2] = static_cast<std::string>(config["DBUser"]);
		dbinfo[3] = static_cast<std::string>(config["DBPass"]);
		dbinfo[4] = static_cast<std::string>(config["DBName"]);

		EOServer<EOClient> server(static_cast<std::string>(config["Host"]), static_cast<int>(config["Port"]), dbinfo, config);

		if (server.State() == Server<EOClient>::Invalid)
		{
			std::puts("There was a problem initializing the server. (Is port 8078 already in use?)");
			std::exit(1);
		}

		std::printf("Listening on %s:%i (0/%i connections)\n", static_cast<std::string>(config["Host"]).c_str(), static_cast<int>(config["Port"]), static_cast<int>(config["MaxConnections"]));

		if (!server.Listen(static_cast<int>(config["MaxConnections"]), static_cast<int>(config["ListenBacklog"])))
		{
			std::puts("Failed to bind, make sure the above port is not already in use.");
			std::exit(1);
		}

		// This also doubles as a check for table existance :P
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
