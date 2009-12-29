
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef EOCLIENT_HPP_INCLUDED
#define EOCLIENT_HPP_INCLUDED

#include "stdafx.h"

#include "eoserver.hpp"
#include "packet.hpp"
#include "socket.hpp"

#define CLIENT_F_FUNC(FUNC) bool Handle_##FUNC(PacketFamily family, PacketAction action, PacketReader &reader, int act)

/**
 * An action the server will execute for the client
 */
struct ActionQueue_Action : public Shared
{
	PacketFamily family;
	PacketAction action;
	PacketReader reader;
	double time;

	ActionQueue_Action(PacketFamily family_, PacketAction action_, PacketReader reader_, double time_) : family(family_), action(action_), reader(reader_), time(time_) {};

	static ActionQueue_Action *ScriptFactory(PacketFamily family, PacketAction action, PacketReader reader, double time) { return new ActionQueue_Action(family, action, reader, time); }

	SCRIPT_REGISTER_REF(ActionQueue_Action)
		SCRIPT_REGISTER_FACTORY("ActionQueue_Action @f(PacketFamily family, PacketAction action, PacketReader reader, double time)", ScriptFactory);

		SCRIPT_REGISTER_VARIABLE("PacketFamily", family);
		SCRIPT_REGISTER_VARIABLE("PacketAction", action);
		SCRIPT_REGISTER_VARIABLE("PacketReader", reader);
		SCRIPT_REGISTER_VARIABLE("double", time);
	SCRIPT_REGISTER_END()
};

/**
 * A list of actions a client needs to eventually have executed for it
 */
class ActionQueue : public Shared
{
	public:
		std::queue<ActionQueue_Action *> queue;

		double next;
		void AddAction(PacketFamily family, PacketAction action, PacketReader reader, double time);
		void Execute();

		ActionQueue() : next(0) {};

		~ActionQueue();

	SCRIPT_REGISTER_REF_DF(ActionQueue)
		SCRIPT_REGISTER_VARIABLE("double", next);
		SCRIPT_REGISTER_FUNCTION("void AddAction(PacketFamily family, PacketAction action, PacketReader reader, double time)", AddAction);
		SCRIPT_REGISTER_FUNCTION("void Execute()", Execute);
	SCRIPT_REGISTER_END()
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
		EOServer *server;
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

		EOClient(EOServer *server_) : Client(server_), server(server_)
		{
			this->Initialize();
		}

		EOClient(SOCKET s, sockaddr_in sa, EOServer *server_) : Client(s, sa, server_), server(server_)
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

	static EOClient *ScriptFactory(EOServer *server) { return new EOClient(server); }

	SCRIPT_REGISTER_REF(EOClient)
		SCRIPT_REGISTER_FACTORY("EOClient @f(EOServer @server)", ScriptFactory);

		SCRIPT_REGISTER_ENUM("PacketState")
			SCRIPT_REGISTER_ENUM_VALUE(ReadLen1);
			SCRIPT_REGISTER_ENUM_VALUE(ReadLen2);
			SCRIPT_REGISTER_ENUM_VALUE(ReadData);
		SCRIPT_REGISTER_ENUM_END()

		SCRIPT_REGISTER_ENUM("ClientState")
			SCRIPT_REGISTER_ENUM_VALUE(Uninitialized);
			SCRIPT_REGISTER_ENUM_VALUE(Initialized);
			SCRIPT_REGISTER_ENUM_VALUE(LoggedIn);
			SCRIPT_REGISTER_ENUM_VALUE(PlayingModal);
			SCRIPT_REGISTER_ENUM_VALUE(Playing);
		SCRIPT_REGISTER_ENUM_END()

		SCRIPT_REGISTER_VARIABLE("EOServer @", server);
		SCRIPT_REGISTER_VARIABLE("int", version);
		SCRIPT_REGISTER_VARIABLE("Player @", player);
		SCRIPT_REGISTER_VARIABLE("uint", id);
		SCRIPT_REGISTER_VARIABLE("bool", needpong);
		SCRIPT_REGISTER_VARIABLE("int", hdid);
		SCRIPT_REGISTER_VARIABLE("ClientState", state);
		SCRIPT_REGISTER_VARIABLE("ActionQueue", queue);
		SCRIPT_REGISTER_VARIABLE("PacketState", packet_state);
		// raw_length
		SCRIPT_REGISTER_VARIABLE("int", length);
		SCRIPT_REGISTER_VARIABLE("string", data);
		SCRIPT_REGISTER_VARIABLE("PacketProcessor", processor);
		SCRIPT_REGISTER_FUNCTION("void Execute(string data)", Execute);
		SCRIPT_REGISTER_FUNCTION("void SendBuilder(PacketBuilder &in)", SendBuilder);
		SCRIPT_REGISTER_FUNCTION("void SendBuilderRaw(PacketBuilder &in)", SendBuilderRaw);
	SCRIPT_REGISTER_END()
};

#endif // EOCLIENT_HPP_INCLUDED
