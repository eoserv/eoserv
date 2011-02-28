
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "eoclient.hpp"

#include "console.hpp"
#include "eoclient.hpp"
#include "eoserver.hpp"
#include "nanohttp.hpp"
#include "packet.hpp"
#include "player.hpp"
#include "timer.hpp"
#include "socket.hpp"
#include "world.hpp"
#include "handlers/handlers.hpp"

#define CLIENT_F_HANDLE(ID,FUNC) \
case ID: \
	result = this->Handle_##FUNC(family, action, reader, false);\
	break

void ActionQueue::AddAction(PacketFamily family, PacketAction action, PacketReader reader, double time)
{
	this->queue.push(new ActionQueue_Action(family, action, reader, time));
}

ActionQueue::~ActionQueue()
{
	while (!this->queue.empty())
	{
		delete this->queue.front();
		this->queue.pop();
	}
}

void EOClient::Initialize()
{
	this->id = this->server()->world->GeneratePlayerID();
	this->length = 0;
	this->packet_state = EOClient::ReadLen1;
	this->state = EOClient::Uninitialized;
	this->player = 0;
	this->version = 0;
	this->needpong = false;
}

void EOClient::Execute(std::string data)
{
	PacketFamily family;
	PacketAction action;

	if (data.length() < 2)
	{
		return;
	}

	data = processor.Decode(data);

	family = static_cast<PacketFamily>(static_cast<unsigned char>(data[1]));
	action = static_cast<PacketAction>(static_cast<unsigned char>(data[0]));

	if (family == PACKET_INTERNAL)
	{
		Console::Wrn("Closing client connection sending a reserved packet ID.");
		this->Close();
		return;
	}

	PacketReader reader(data.substr(2));

	if (family != PACKET_F_INIT)
	{
		reader.GetChar(); // Ordering Byte
	}

	Handlers::Handle(family, action, this, reader);
}

void EOClient::Send(const PacketBuilder &builder)
{
	std::string packet(builder);

	std::array<unsigned char, 2> id = PacketProcessor::EPID(builder.GetID());

	if (id[1] != PACKET_F_INIT)
		Client::Send(this->processor.Encode(packet));
	else
		Client::Send(packet);
}

EOClient::~EOClient()
{
	if (this->player)
	{
		this->player->Logout();
	}
}
