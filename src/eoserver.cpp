
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "eoserver.hpp"

#include <string>

#include <pthread.h>

#include "world.hpp"

#include "socket.hpp"
#include "packet.hpp"
#include "eoclient.hpp"
#include "timer.hpp"
#include "nanohttp.hpp"
#include "util.hpp"
#include "sln.hpp"

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

	UTIL_LIST_FOREACH_ALL(server->clients, EOClient *, client)
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

	UTIL_LIST_FOREACH_ALL(server->clients, EOClient *, client)
	{
		std::size_t size = client->queue.size();

		if (size > 40)
		{
#ifdef DEBUG
			std::puts("Client was disconnected for filling up the action queue");
#endif // DEBUG
			client->Close();
			continue;
		}

		if (size != 0 && client->queue.next <= now)
		{
			ActionQueue_Action *action = client->queue.front();
			client->queue.pop();

			bool result;

			switch (action->family)
			{
				QUEUE_F_HANDLE(PACKET_F_INIT,Init);
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

void EOServer::Initialize(util::array<std::string, 5> dbinfo, const Config &eoserv_config, const Config &admin_config)
{
	this->world = new World(dbinfo, eoserv_config, admin_config);
	this->world->timer.Register(new TimeEvent(server_ping_all, this, 60.0, Timer::FOREVER, true));
	this->world->timer.Register(new TimeEvent(server_pump_queue, this, 0.001, Timer::FOREVER, true));

	this->world->server = this;

	if (static_cast<int>(this->world->config["SLN"]))
	{
		this->sln = new SLN(this);
	}
	else
	{
		this->sln = 0;
	}
}

void EOServer::AddBan(std::string username, IPAddress address, std::string hdid, double duration)
{
	double now = Timer::GetTime();
	restart_loop:
	UTIL_VECTOR_IFOREACH(this->bans.begin(), this->bans.end(), EOServer_Ban, ban)
	{
		if (ban->expires < now)
		{
			this->bans.erase(ban);
			goto restart_loop;
		}
	}
	EOServer_Ban newban;
	newban.username = username;
	newban.address = address;
	newban.hdid = hdid;
	newban.expires = now + duration;
	bans.push_back(newban);
}

bool EOServer::UsernameBanned(std::string username)
{
	double now = Timer::GetTime();
	UTIL_VECTOR_FOREACH_ALL(this->bans, EOServer_Ban, ban)
	{
		if (ban.expires > now && ban.username == username)
		{
			return true;
		}
	}

	return false;
}

bool EOServer::AddressBanned(IPAddress address)
{
	double now = Timer::GetTime();
	UTIL_VECTOR_FOREACH_ALL(this->bans, EOServer_Ban, ban)
	{
		if (ban.expires > now && ban.address == address)
		{
			return true;
		}
	}

	return false;
}

bool EOServer::HDIDBanned(std::string hdid)
{
	double now = Timer::GetTime();
	UTIL_VECTOR_FOREACH_ALL(this->bans, EOServer_Ban, ban)
	{
		if (ban.expires > now && ban.hdid == hdid)
		{
			return true;
		}
	}

	return false;
}

EOServer::~EOServer()
{
	if (this->sln)
	{
		delete this->sln;
	}

	delete this->world;
}
