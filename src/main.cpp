
#include "config.hpp"
#include "socket.hpp"
#include "eoclient.hpp"
#include "packet.hpp"
#include <climits>

bool running = true;
Config config("config.ini");

int main(void)
{
	puts("\
                          ___ ___  ___ ___ _____   __\n\
   EOSERV Version 0.2.0  | __/ _ \\/ __| __| _ \\ \\ / /    http://eoserv.net/\n\
=========================| _| (_) \\__ \\ _||   /\\ ` /===========================\n\
       07 Feb 2009       |___\\___/|___/___|_|_\\ \\_/    sausage@tehsausage.com\n");

	Server<EOClient> server(static_cast<std::string>(config["Host"]), static_cast<int>(config["Port"]));
	if (server.State() == Server<EOClient>::Invalid)
	{
		puts("There was a problem initializing the server.");
#ifdef WIN32
		puts("Is Winsock available and up-to-date? (EOSERV requires Winsock 2.0 or higher)");
#endif // WIN32
		exit(1);
	}

	printf("Listening on %s:%i (0/%i connections)\n", static_cast<std::string>(config["Host"]).c_str(), static_cast<int>(config["Port"]), static_cast<int>(config["MaxConnections"]));

	if (!server.Listen(static_cast<int>(config["MaxConnections"]), static_cast<int>(config["ListenBacklog"])))
	{
		puts("Failed to bind, make sure the above port is not already in use.");
		exit(1);
	}

	while (running)
	{
		Client *newclient;
		if ((newclient = server.Poll()) != NULL)
		{
			printf("New connection from %s (%i/%i connections)\n", static_cast<std::string>(newclient->GetRemoteAddr()).c_str(), server.Connections(), server.MaxConnections());
		}

		std::list<EOClient *> active_clients = server.Select(1);

		for (std::list<EOClient *>::iterator ci = active_clients.begin(); ci != active_clients.end(); ++ci)
		{
			EOClient *cl = *ci;
			std::string data;
			int done = false;

			data = cl->Recv((cl->state == EOClient::ReadData)?cl->length:1);

			while (data.length() > 0 && !done)
			{
				switch (cl->state)
				{
					case EOClient::ReadLen1:
						cl->raw_length[0] = data[0];
						data.erase(0,1);
						cl->state = EOClient::ReadLen2;
						printf("First length: %u\n",cl->raw_length[0]);
						break;

					case EOClient::ReadLen2:
						cl->raw_length[1] = data[0];
						data.erase(0,1);
						cl->length = PacketProcessor::Number(cl->raw_length[0], cl->raw_length[1]);
						cl->state = EOClient::ReadData;
						printf("Second length: %u  (len = %i)\n",cl->raw_length[1], cl->length);
						break;

					case EOClient::ReadData:
						printf("Recieving message...\n");
						int oldlength = cl->data.length();
						cl->data += data.substr(0, cl->length);
						cl->length -= cl->data.length() - oldlength;
						if (cl->length == 0)
						{
							cl->Execute(cl->data);

							cl->data.erase();
							cl->state = EOClient::ReadLen1;
							printf("Full message recieved.\n");

							done = true;
						}
						break;

					default:
						// If the code ever gets here, something is broken, so we just reset it's state.
						data.erase();
						cl->data.erase();
						cl->state = EOClient::ReadLen1;
				}
			}
		}

		server.BuryTheDead();
	}

	return 0;
}
