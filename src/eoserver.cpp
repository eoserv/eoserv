
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

#define CLIENT_F_HANDLE(ID,FUNC) \
case ID: \
	result = this->Handle_##FUNC(family, action, reader, false);\
	break

#define QUEUE_F_HANDLE(ID,FUNC) \
case ID: \
	result = client->Handle_##FUNC(action->family, action->action, action->reader, true);\
	break

void server_ping_all(void *server_void)
{
	EOServer *server = static_cast<EOServer *>(server_void);

	PacketBuilder builder;

	builder.SetID(PACKET_CONNECTION, PACKET_PLAYER);
	builder.AddShort(0);
	builder.AddChar(0);

	UTIL_PTR_LIST_FOREACH(server->clients, EOClient, client)
	{
		if (client->needpong)
		{
			client->Close();
		}
		else
		{
			client->needpong = true;
			client->SendBuilder(builder);
		}
	}
}

void server_pump_queue(void *server_void)
{
	EOServer *server = static_cast<EOServer *>(server_void);
	double now = Timer::GetTime();

	UTIL_PTR_LIST_FOREACH(server->clients, EOClient, client)
	{
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

			bool result;

			switch (action->family)
			{
				QUEUE_F_HANDLE(PACKET_F_INIT,Init);
				QUEUE_F_HANDLE(PACKET_INTERNAL,Internal);
				QUEUE_F_HANDLE(PACKET_CONNECTION,Connection);
				QUEUE_F_HANDLE(PACKET_ACCOUNT,Account);
				QUEUE_F_HANDLE(PACKET_CHARACTER,Character);
				QUEUE_F_HANDLE(PACKET_LOGIN,Login);
				QUEUE_F_HANDLE(PACKET_WELCOME,Welcome);
				QUEUE_F_HANDLE(PACKET_WALK,Walk);
				QUEUE_F_HANDLE(PACKET_FACE,Face);
				QUEUE_F_HANDLE(PACKET_CHAIR,Chair);
				QUEUE_F_HANDLE(PACKET_EMOTE,Emote);
				QUEUE_F_HANDLE(PACKET_ATTACK,Attack);
				QUEUE_F_HANDLE(PACKET_SHOP,Shop);
				QUEUE_F_HANDLE(PACKET_ITEM,Item);
				QUEUE_F_HANDLE(PACKET_STATSKILL,StatSkill);
				QUEUE_F_HANDLE(PACKET_GLOBAL,Global);
				QUEUE_F_HANDLE(PACKET_TALK,Talk);
				QUEUE_F_HANDLE(PACKET_WARP,Warp);
				QUEUE_F_HANDLE(PACKET_JUKEBOX,Jukebox);
				QUEUE_F_HANDLE(PACKET_PLAYERS,Players);
				QUEUE_F_HANDLE(PACKET_PARTY,Party);
				QUEUE_F_HANDLE(PACKET_REFRESH,Refresh);
				QUEUE_F_HANDLE(PACKET_PAPERDOLL,Paperdoll);
				QUEUE_F_HANDLE(PACKET_TRADE,Trade);
				QUEUE_F_HANDLE(PACKET_CHEST,Chest);
				QUEUE_F_HANDLE(PACKET_DOOR,Door);
				QUEUE_F_HANDLE(PACKET_PING,Ping);
				QUEUE_F_HANDLE(PACKET_BANK,Bank);
				QUEUE_F_HANDLE(PACKET_LOCKER,Locker);
				QUEUE_F_HANDLE(PACKET_GUILD,Guild);
				QUEUE_F_HANDLE(PACKET_SIT,Sit);
				QUEUE_F_HANDLE(PACKET_BOARD,Board);
				//QUEUE_F_HANDLE(PACKET_ARENA,Arena);
				QUEUE_F_HANDLE(PACKET_ADMININTERACT,AdminInteract);
				QUEUE_F_HANDLE(PACKET_CITIZEN,Citizen);
				//QUEUE_F_HANDLE(PACKET_QUEST,Quest);
				QUEUE_F_HANDLE(PACKET_BOOK,Book);
				default: ; // Keep the compiler quiet until all packet types are handled
			}

			client->queue.next = now + action->time;

			delete action;
		}
	}
}

Client *EOServer::ClientFactory(SOCKET sock, sockaddr_in sin)
{
	return new EOClient(sock, sin, this);
}

void EOServer::Initialize(util::array<std::string, 5> dbinfo, const Config &eoserv_config, const Config &admin_config)
{
	this->world = new World(dbinfo, eoserv_config, admin_config);
	this->world->timer.Register(new TimeEvent(server_ping_all, this, 60.0, Timer::FOREVER, true));
	this->world->timer.Register(new TimeEvent(server_pump_queue, this, 0.001, Timer::FOREVER, true));

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

EOServer::~EOServer()
{
	if (this->sln)
	{
		this->sln->Release();
	}

	this->world->Release();
}
