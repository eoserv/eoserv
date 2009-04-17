
#include "eoclient.hpp"

#include <string>

#include "socket.hpp"
#include "packet.hpp"
#include "eoclient.hpp"
#include "eoserv.hpp"
#include "timer.hpp"
#include "nanohttp.hpp"

#define CLIENT_F_HANDLE(ID,FUNC) \
case ID: \
	result = this->Handle_##FUNC(action,reader);\
	break

void server_ping_all(void *server_void)
{
	EOServer *server = static_cast<EOServer *>(server_void);

	PacketBuilder builder;

	builder.SetID(PACKET_CONNECTION, PACKET_PLAYER);
	builder.AddShort(0);
	builder.AddChar(0);

	std::list<int> x;

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

HTTP *sln_http;
TimeEvent *sln_tick_request_timer;

void sln_request(void *server_void)
{
	EOServer *server = static_cast<EOServer *>(server_void);

	if (sln_tick_request_timer != 0)
	{
		return;
	}

	std::string url = eoserv_config["SLNURL"];
	url += "check?software=EOSERV";
	url += std::string("&retry=") + static_cast<std::string>(eoserv_config["SLNPeriod"]);
	if (static_cast<std::string>(eoserv_config["SLNHost"]).length() > 0)
	{
		url += std::string("&host=") + HTTP::URLEncode(eoserv_config["SLNHost"]);
	}
	url += std::string("&port=") + HTTP::URLEncode(eoserv_config["Port"]);
	url += std::string("&name=") + HTTP::URLEncode(eoserv_config["ServerName"]);
	if (static_cast<std::string>(eoserv_config["SLNSite"]).length() > 0)
	{
		url += std::string("&url=") + HTTP::URLEncode(eoserv_config["SLNSite"]);
	}
	if (static_cast<std::string>(eoserv_config["SLNZone"]).length() > 0)
	{
		url += std::string("&zone=") + HTTP::URLEncode(eoserv_config["SLNZone"]);
	}

	sln_http = HTTP::RequestURL(url);
	sln_tick_request_timer = new TimeEvent(sln_tick_request, server_void, 0.01, Timer::FOREVER, false);
	server->world->timer.Register(sln_tick_request_timer);
}

void sln_tick_request(void *server_void)
{
	EOServer *server = static_cast<EOServer *>(server_void);

	if (sln_http == 0)
	{
		return;
	}

	sln_http->Tick(0);

	if (sln_http->Done())
	{
		delete sln_http;
		sln_http = 0;
		if (sln_tick_request_timer != 0)
		{
			server->world->timer.Unregister(sln_tick_request_timer);
		}
		sln_tick_request_timer = 0;
		if (static_cast<int>(eoserv_config["SLN"]))
		{
			server->world->timer.Register(new TimeEvent(sln_request, server_void, eoserv_config["SLNPeriod"], 1, true));
		}
	}
}

void EOServer::Initialize(util::array<std::string, 5> dbinfo, Config config)
{
	this->world = new World(dbinfo, config);
	this->world->timer.Register(new TimeEvent(server_ping_all, this, 60.0, Timer::FOREVER));
	if (static_cast<int>(eoserv_config["SLN"]))
	{
		sln_tick_request_timer = 0;
		sln_request(this);
	}
	this->world->server = this;
}

void EOServer::AddBan(std::string username, IPAddress address, std::string hdid, double duration)
{
	double now = Timer::GetTime();
	restart_loop:
	UTIL_LIST_IFOREACH(this->bans.begin(), this->bans.end(), EOServer_Ban, ban)
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
	UTIL_LIST_FOREACH_ALL(this->bans, EOServer_Ban, ban)
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
	UTIL_LIST_FOREACH_ALL(this->bans, EOServer_Ban, ban)
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
	UTIL_LIST_FOREACH_ALL(this->bans, EOServer_Ban, ban)
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
	delete this->world;
}

void EOClient::Initialize()
{
	this->id = the_world->GeneratePlayerID();
	this->length = 0;
	this->state = EOClient::ReadLen1;
	this->player = 0;
	this->version = 0;
	this->needpong = false;
	this->init = false;
}

void EOClient::Execute(std::string data)
{
	unsigned char family;
	unsigned char action;

	if (data.length() < 2)
	{
		return;
	}

	data = processor.Decode(data);

	family = data[1];
	action = data[0];

	PacketReader reader(data.substr(2));

	bool result = false;

	if (family != PACKET_INIT)
	{
		reader.GetChar(); // Ordering Byte
	}

	if (!this->init && family != PACKET_INIT && family != PACKET_PLAYERS)
	{
		this->Close();
		return;
	}

	switch (family)
	{
		CLIENT_F_HANDLE(PACKET_INIT,Init);
		CLIENT_F_HANDLE(PACKET_CONNECTION,Connection);
		CLIENT_F_HANDLE(PACKET_ACCOUNT,Account);
		CLIENT_F_HANDLE(PACKET_CHARACTER,Character);
		CLIENT_F_HANDLE(PACKET_LOGIN,Login);
		CLIENT_F_HANDLE(PACKET_WELCOME,Welcome);
		CLIENT_F_HANDLE(PACKET_WALK,Walk);
		CLIENT_F_HANDLE(PACKET_FACE,Face);
		CLIENT_F_HANDLE(PACKET_CHAIR,Chair);
		CLIENT_F_HANDLE(PACKET_EMOTE,Emote);
		CLIENT_F_HANDLE(PACKET_ATTACK,Attack);
		CLIENT_F_HANDLE(PACKET_SHOP,Shop);
		CLIENT_F_HANDLE(PACKET_ITEM,Item);
		CLIENT_F_HANDLE(PACKET_SKILLMASTER,SkillMaster);
		CLIENT_F_HANDLE(PACKET_GLOBAL,Global);
		CLIENT_F_HANDLE(PACKET_TALK,Talk);
		CLIENT_F_HANDLE(PACKET_WARP,Warp);
		CLIENT_F_HANDLE(PACKET_JUKEBOX,Jukebox);
		CLIENT_F_HANDLE(PACKET_PLAYERS,Players);
		CLIENT_F_HANDLE(PACKET_PARTY,Party);
		CLIENT_F_HANDLE(PACKET_REFRESH,Refresh);
		CLIENT_F_HANDLE(PACKET_PAPERDOLL,Paperdoll);
		CLIENT_F_HANDLE(PACKET_TRADE,Trade);
		CLIENT_F_HANDLE(PACKET_CHEST,Chest);
		CLIENT_F_HANDLE(PACKET_DOOR,Door);
		CLIENT_F_HANDLE(PACKET_PING,Ping);
		CLIENT_F_HANDLE(PACKET_BANK,Bank);
		CLIENT_F_HANDLE(PACKET_LOCKER,Locker);
		CLIENT_F_HANDLE(PACKET_GUILD,Guild);
		CLIENT_F_HANDLE(PACKET_SIT,Sit);
		CLIENT_F_HANDLE(PACKET_BOARD,Board);
		//CLIENT_F_HANDLE(PACKET_ARENA,Arena);
		CLIENT_F_HANDLE(PACKET_ADMININTERACT,AdminInteract);
		CLIENT_F_HANDLE(PACKET_CITIZEN,Citizen);
		//CLIENT_F_HANDLE(PACKET_QUEST,Quest);
		CLIENT_F_HANDLE(PACKET_BOOK,Book);
	}

#ifdef DEBUG
	if (family != PACKET_CONNECTION || family != PACKET_NET)
	{
		std::printf("Packet %s[%i]_%s[%i] from %s\n", PacketProcessor::GetFamilyName(family).c_str(), family, PacketProcessor::GetActionName(action).c_str(), action, static_cast<std::string>(this->GetRemoteAddr()).c_str());
	}
#endif
}

void EOClient::SendBuilder(PacketBuilder &builder)
{
	std::string packet = static_cast<std::string >(builder);
	this->Send(this->processor.Encode(packet));
}

EOClient::~EOClient()
{
	if (this->player)
	{
		delete this->player; // Player handles removing himself from the world
	}
}
