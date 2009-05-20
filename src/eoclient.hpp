#ifndef EOCLIENT_HPP_INCLUDED
#define EOCLIENT_HPP_INCLUDED

#include <string>
#include <cstddef>
#include <vector>
#include <queue>

class EOServer_Ban;
class ActionQueue_Action;

class EOServer;
class EOClient;
class ActionQueue;

#include "socket.hpp"
#include "packet.hpp"
#include "eoserv.hpp"
#include "util.hpp"
#include "config.hpp"

#define CLIENT_F_FUNC(FUNC) bool Handle_##FUNC(PacketFamily family, PacketAction action, PacketReader &reader, int act)

void server_ping_all(void *server_void);
void sln_request(void *server_void);
void *real_sln_request(void *server_void);
void sln_tick_request(void *server_void);
void server_pump_queue(void *server_void);

/**
 * An action the server will execute for the client
 */
struct ActionQueue_Action
{
	PacketFamily family;
	PacketAction action;
	PacketReader reader;
	double time;

	ActionQueue_Action(PacketFamily family, PacketAction action, PacketReader reader, double time) : family(family), action(action), reader(reader), time(time) {};
};

/**
 * A list of actions a client needs to eventually have executed for it
 */
class ActionQueue : public std::queue<ActionQueue_Action *>
{
	public:
		double next;
		void Execute();

		ActionQueue() : next(0) {};

		~ActionQueue();
};

/**
 * Information about a temporary in-memory ban
 */
struct EOServer_Ban
{
	std::string username;
	IPAddress address;
	std::string hdid;
	double expires;
};

/**
 * A server which accepts connections and creates EOClient instances from them
 */
class EOServer : public Server<EOClient>
{
	private:
		void Initialize(util::array<std::string, 5> dbinfo, Config config);
		EOServer(){};
		std::vector<EOServer_Ban> bans;

	public:
		World *world;

		EOServer(IPAddress addr, unsigned short port, util::array<std::string, 5> dbinfo, Config config) : Server<EOClient>(addr, port)
		{
			this->Initialize(dbinfo, config);
		}

		void AddBan(std::string username, IPAddress address, std::string hdid, double duration);

		bool UsernameBanned(std::string username);
		bool AddressBanned(IPAddress address);
		bool HDIDBanned(std::string hdid);

		~EOServer();
};

/**
 * A connection between an EO Client and EOSERV
 */
class EOClient : public Client
{
	private:
		void Initialize();
		EOClient();

	public:
		int version;
		Player *player;
		unsigned int id;
		bool needpong;
		std::string hdid;

		ActionQueue queue;

		enum PacketState
		{
			ReadLen1,
			ReadLen2,
			ReadData
		};

		enum ClientState
		{
			Uninitialized,
			Initialized,
			LoggedIn,
			PlayingModal,
			Playing
		};

		PacketState packet_state;
		ClientState state;
		unsigned char raw_length[2];
		unsigned int length;
		std::string data;

		PacketProcessor processor;

		EOClient(void *server) : Client(server)
		{
			this->Initialize();
		}

		EOClient(SOCKET s, sockaddr_in sa, void *server) : Client(s, sa, server)
		{
			this->Initialize();
		}

		void Execute(std::string data);

		void SendBuilder(PacketBuilder &packet);

// Stop doxygen generating a gigantic list of functions
#ifndef DOXYGEN
		CLIENT_F_FUNC(Init);
		CLIENT_F_FUNC(Connection);
		CLIENT_F_FUNC(Account);
		CLIENT_F_FUNC(Character);
		CLIENT_F_FUNC(Login);
		CLIENT_F_FUNC(Welcome);
		CLIENT_F_FUNC(Walk);
		CLIENT_F_FUNC(Face);
		CLIENT_F_FUNC(Chair);
		CLIENT_F_FUNC(Emote);
		CLIENT_F_FUNC(Attack);
		CLIENT_F_FUNC(Shop);
		CLIENT_F_FUNC(Item);
		CLIENT_F_FUNC(StatSkill);
		CLIENT_F_FUNC(Global);
		CLIENT_F_FUNC(Talk);
		CLIENT_F_FUNC(Warp);
		CLIENT_F_FUNC(Jukebox);
		CLIENT_F_FUNC(Players);
		CLIENT_F_FUNC(Party);
		CLIENT_F_FUNC(Refresh);
		CLIENT_F_FUNC(Paperdoll);
		CLIENT_F_FUNC(Trade);
		CLIENT_F_FUNC(Chest);
		CLIENT_F_FUNC(Door);
		CLIENT_F_FUNC(Ping);
		CLIENT_F_FUNC(Bank);
		CLIENT_F_FUNC(Locker);
		CLIENT_F_FUNC(Guild);
		CLIENT_F_FUNC(Sit);
		CLIENT_F_FUNC(Board);
		//CLIENT_F_FUNC(Arena);
		CLIENT_F_FUNC(AdminInteract);
		CLIENT_F_FUNC(Citizen);
		//CLIENT_F_FUNC(Quest);
		CLIENT_F_FUNC(Book);
#endif // DOXYGEN

		virtual ~EOClient();
};

#endif // EOCLIENT_HPP_INCLUDED
