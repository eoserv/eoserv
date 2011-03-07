
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

/**
 * An action the server will execute for the client
 */
struct ActionQueue_Action
{
	PacketReader reader;
	double time;
	bool auto_queue;

	ActionQueue_Action(PacketReader reader_, double time_, bool auto_queue_ = false)
		: reader(reader_)
		, time(time_)
		, auto_queue(auto_queue_)
	{ }
};

/**
 * A list of actions a client needs to eventually have executed for it
 */
class ActionQueue
{
	public:
		std::queue<ActionQueue_Action *> queue;

		double next;
		void AddAction(PacketReader reader, double time, bool auto_queue = false);

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

		void Tick();

		void Execute(std::string data);

		void Send(const PacketBuilder &packet);

		~EOClient();
};

#endif // EOCLIENT_HPP_INCLUDED
