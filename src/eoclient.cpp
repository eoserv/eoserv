
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

#define CLIENT_F_HANDLE(ID,FUNC) \
case ID: \
	result = this->Handle_##FUNC(family, action, reader, false);\
	break

ActionQueue::~ActionQueue()
{
	while (!this->empty())
	{
		ActionQueue_Action *action = this->front();
		this->pop();
		delete action;
	}
}

void EOClient::Initialize()
{
	this->id = this->server->world->GeneratePlayerID();
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

	bool result = false;

	if (family != PACKET_F_INIT)
	{
		reader.GetChar(); // Ordering Byte
	}

	if (this->state < EOClient::Initialized && family != PACKET_F_INIT && family != PACKET_PLAYERS)
	{
		Console::Wrn("Closing client connection sending a non-init packet before init.");
		this->Close();
		return;
	}

	switch (family)
	{
		CLIENT_F_HANDLE(PACKET_F_INIT,Init);
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
		CLIENT_F_HANDLE(PACKET_STATSKILL,StatSkill);
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
		default: ; // Keep the compiler quiet until all packet types are handled
	}

#ifdef DEBUG
	//if (family != PACKET_CONNECTION || action != PACKET_NET)
	{
		Console::Dbg("Packet %s[%i]_%s[%i] from %s", PacketProcessor::GetFamilyName(family).c_str(), family, PacketProcessor::GetActionName(action).c_str(), action, static_cast<std::string>(this->GetRemoteAddr()).c_str());
	}
#endif
}

void EOClient::SendBuilder(PacketBuilder &builder)
{
	std::string packet(builder);
	this->Send(this->processor.Encode(packet));
}

EOClient::~EOClient()
{
	if (this->player)
	{
		this->player->Logout();
	}
}
