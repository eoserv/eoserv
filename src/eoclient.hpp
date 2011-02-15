
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef EOCLIENT_HPP_INCLUDED
#define EOCLIENT_HPP_INCLUDED

#include "fwd/eoclient.hpp"

#include <queue>

#include "socket.hpp"

#include "fwd/character.hpp"
#include "fwd/player.hpp"
#include "eoserver.hpp"
#include "packet.hpp"

#define CLIENT_F_FUNC(FUNC) bool Handle_##FUNC(PacketFamily family, PacketAction action, PacketReader &reader, int act)

/**
 * An action the server will execute for the client
 */
struct ActionQueue_Action
{
	PacketFamily family;
	PacketAction action;
	PacketReader reader;
	double time;

	ActionQueue_Action(PacketFamily family_, PacketAction action_, PacketReader reader_, double time_) : family(family_), action(action_), reader(reader_), time(time_) {};
};

/**
 * A list of actions a client needs to eventually have executed for it
 */
class ActionQueue
{
	public:
		std::queue<ActionQueue_Action *> queue;

		double next;
		void AddAction(PacketFamily family, PacketAction action, PacketReader reader, double time);

		ActionQueue() : next(0) {};

		~ActionQueue();
};

/**
 * A connection between an EO Client and EOSERV
 */
class EOClient : public Client
{
	public:
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

	private:
		void Initialize();
		EOClient();

	public:
		EOServer *server() { return static_cast<EOServer *>(Client::server); };
		int version;
		Player *player;
		unsigned int id;
		bool needpong;
		int hdid;
		ClientState state;

		ActionQueue queue;

		PacketState packet_state;
		unsigned char raw_length[2];
		unsigned int length;
		std::string data;

		PacketProcessor processor;

		EOClient(EOServer *server_) : Client(server_)
		{
			this->Initialize();
		}

		EOClient(const Socket &sock, EOServer *server_) : Client(sock, server_)
		{
			this->Initialize();
		}

		void Execute(std::string data);

		void SendBuilder(const PacketBuilder &packet);
		void SendBuilderRaw(const PacketBuilder &packet);

// Stop doxygen generating a gigantic list of functions
#ifndef DOXYGEN
		CLIENT_F_FUNC(Internal);
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
		CLIENT_F_FUNC(Barber);
		CLIENT_F_FUNC(Guild);
		CLIENT_F_FUNC(Sit);
		CLIENT_F_FUNC(Board);
		//CLIENT_F_FUNC(Arena);
		CLIENT_F_FUNC(AdminInteract);
		CLIENT_F_FUNC(Citizen);
		//CLIENT_F_FUNC(Quest);
		CLIENT_F_FUNC(Book);
#endif // DOXYGEN

		~EOClient();
};

#endif // EOCLIENT_HPP_INCLUDED
