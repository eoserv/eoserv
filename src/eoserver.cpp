
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "eoserver.hpp"

#include "console.hpp"
#include "eoclient.hpp"
#include "nanohttp.hpp"
#include "packet.hpp"
#include "sln.hpp"
#include "socket.hpp"
#include "timer.hpp"
#include "world.hpp"
#include "handlers/handlers.hpp"

void server_ping_all(void *server_void)
{
	EOServer *server = static_cast<EOServer *>(server_void);

	PacketBuilder builder(PACKET_CONNECTION, PACKET_PLAYER, 3);
	builder.AddShort(0);
	builder.AddChar(0);

	UTIL_FOREACH(server->clients, rawclient)
	{
		EOClient *client = static_cast<EOClient *>(rawclient);

		if (client->needpong)
		{
			client->Close();
		}
		else
		{
			client->needpong = true;
			client->Send(builder);
		}
	}
}

void server_pump_queue(void *server_void)
{
	EOServer *server = static_cast<EOServer *>(server_void);
	double now = Timer::GetTime();

	UTIL_FOREACH(server->clients, rawclient)
	{
		EOClient *client = static_cast<EOClient *>(rawclient);

		std::size_t size = client->queue.queue.size();

		if (size > 40)
		{
#ifdef DEBUG
			Console::Wrn("Client was disconnected for filling up the action queue");
#endif // DEBUG
			client->Close();
			continue;
		}

		if (size != 0 && client->queue.next <= now)
		{
			ActionQueue_Action *action = client->queue.queue.front();
			client->queue.queue.pop();

			try
			{
				Handlers::Handle(action->reader.Family(), action->reader.Action(), client, action->reader, !action->auto_queue);
			}
			catch (Socket_Exception& e)
			{
				Console::Err("Client caused an exception and was closed: %s.", static_cast<std::string>(client->GetRemoteAddr()).c_str());
				Console::Err("%s: %s", e.what(), e.error());
				client->Close();
			}
			catch (Database_Exception& e)
			{
				Console::Err("Client caused an exception and was closed: %s.", static_cast<std::string>(client->GetRemoteAddr()).c_str());
				Console::Err("%s: %s", e.what(), e.error());
				client->Close();
			}
			catch (std::runtime_error& e)
			{
				Console::Err("Client caused an exception and was closed: %s.", static_cast<std::string>(client->GetRemoteAddr()).c_str());
				Console::Err("Runtime Error: %s", e.what());
				client->Close();
			}
			catch (std::logic_error& e)
			{
				Console::Err("Client caused an exception and was closed: %s.", static_cast<std::string>(client->GetRemoteAddr()).c_str());
				Console::Err("Logic Error: %s", e.what());
				client->Close();
			}
			catch (std::exception& e)
			{
				Console::Err("Client caused an exception and was closed: %s.", static_cast<std::string>(client->GetRemoteAddr()).c_str());
				Console::Err("Uncaught Exception: %s", e.what());
				client->Close();
			}
			catch (...)
			{
				Console::Err("Client caused an exception and was closed: %s.", static_cast<std::string>(client->GetRemoteAddr()).c_str());
				client->Close();
			}

			client->queue.next = now + action->time;

			delete action;
		}
	}
}

void EOServer::Initialize(std::array<std::string, 6> dbinfo, const Config &eoserv_config, const Config &admin_config)
{
	this->world = new World(dbinfo, eoserv_config, admin_config);

	TimeEvent *event = new TimeEvent(server_ping_all, this, 60.0, Timer::FOREVER);
	this->world->timer.Register(event);

	event = new TimeEvent(server_pump_queue, this, 0.001, Timer::FOREVER);
	this->world->timer.Register(event);

	this->world->server = this;

	if (this->world->config["SLN"])
	{
		this->sln = new SLN(this);
	}
	else
	{
		this->sln = 0;
	}

	this->start = Timer::GetTime();
}

Client *EOServer::ClientFactory(const Socket &sock)
{
	 return new EOClient(sock, this);
}

void EOServer::Tick()
{
	std::vector<Client *> *active_clients = 0;
	EOClient *newclient = static_cast<EOClient *>(this->Poll());

	if (newclient)
	{
		int ip_connections = 0;
		bool throttle = false;
		IPAddress remote_addr = newclient->GetRemoteAddr();

		const int reconnect_limit = int(this->world->config["IPReconnectLimit"]);
		const int max_per_ip = int(this->world->config["MaxConnectionsPerIP"]);

		UTIL_IFOREACH(connection_log, connection)
		{
			if (connection->second + reconnect_limit < Timer::GetTime())
			{
				connection = connection_log.erase(connection);

				if (connection == connection_log.end())
					break;

				continue;
			}

			if (connection->first == remote_addr)
			{
				throttle = true;
			}
		}

		UTIL_FOREACH(this->clients, client)
		{
			if (client->GetRemoteAddr() == newclient->GetRemoteAddr())
			{
				++ip_connections;
			}
		}

		if (throttle)
		{
			Console::Wrn("Connection from %s was rejected (reconnecting too fast)", std::string(newclient->GetRemoteAddr()).c_str());
			newclient->Close(true);
		}
		else if (max_per_ip != 0 && ip_connections > max_per_ip)
		{
			Console::Wrn("Connection from %s was rejected (too many connections from this address)", std::string(remote_addr).c_str());
			newclient->Close(true);
		}
		else
		{
			connection_log[remote_addr] = Timer::GetTime();
			Console::Out("New connection from %s (%i/%i connections)", std::string(newclient->GetRemoteAddr()).c_str(), this->Connections(), this->MaxConnections());
		}
	}

	try
	{
		active_clients = this->Select(0.001);
	}
	catch (Socket_SelectFailed &e)
	{
		if (errno != EINTR)
			throw;
	}

	if (active_clients)
	{
		UTIL_FOREACH(*active_clients, client)
		{
			EOClient *eoclient = static_cast<EOClient *>(client);
			eoclient->Tick();
		}

		active_clients->clear();
	}

	this->BuryTheDead();

	this->world->timer.Tick();
}

EOServer::~EOServer()
{
	delete this->sln;
	delete this->world;
}
