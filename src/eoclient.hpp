#ifndef EOCLIENT_HPP_INCLUDED
#define EOCLIENT_HPP_INCLUDED

#include <cstdio>
#include <cstddef>
#include <stdint.h>

template <class T> class EOServer;
class EOClient;

#include "socket.hpp"
#include "packet.hpp"
#include "eoserv.hpp"

#define CLIENT_F_FUNC(FUNC) bool Handle_##FUNC(int action, PacketReader &reader)

template <class T> class EOServer : public Server<T>
{
	private:
		void Initialize() { this->world = new World; }

	public:
		World *world;

		EOServer() : Server<T>() { this->Initialize(); };
		EOServer(IPAddress addr, uint16_t port) : Server<T>(addr, port) { this->Initialize(); };
};

class EOClient : public Client
{
	private:
		void Initialize();
		EOClient();

	public:
		Player *player;

		enum State
		{
			ReadLen1,
			ReadLen2,
			ReadData
		};

		int state;
		uint8_t raw_length[2];
		uint32_t length;
		std::string data;

		PacketProcessor processor;
		EOClient(void *server) : Client(server) { this->Initialize(); };
		EOClient(SOCKET s, sockaddr_in sa, void *server) : Client(s, sa, server) { this->Initialize(); };

		void Execute(std::string data);

		void SendBuilder(PacketBuilder &packet);

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
		CLIENT_F_FUNC(SkillMaster);
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

};

#endif // EOCLIENT_HPP_INCLUDED
