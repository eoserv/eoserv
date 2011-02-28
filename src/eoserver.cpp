
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

	PacketBuilder builder;

	builder.SetID(PACKET_CONNECTION, PACKET_PLAYER);
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

			Handlers::Handle(action->family, action->action, client, action->reader, true);

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

EOServer::~EOServer()
{
	delete this->sln;
	delete this->world;
}
