
#include "eoclient.hpp"

#include <string>

#include "socket.hpp"
#include "packet.hpp"
#include "eoclient.hpp"

#define CLIENT_F_HANDLE(ID,FUNC) \
case ID: \
	result = this->Handle_##FUNC(action,reader);\
	break

void EOClient::Initialize()
{
	this->length = 0;
	this->state = 0;
	this->player = new Player;
	this->player->client = this;
	this->player->world = static_cast<EOServer<EOClient> *>(this->server)->world;
	this->player->world->players.push_back(this->player);
}

void EOClient::Execute(std::string data)
{
	unsigned char family;
	unsigned char action;

	data = processor.Decode(data);

	family = data[1];
	action = data[0];

	printf("%s[%i]_%s[%i] from %s\n", PacketProcessor::GetFamilyName(family).c_str(), family, PacketProcessor::GetActionName(action).c_str(), action, static_cast<std::string>(this->GetRemoteAddr()).c_str());

	PacketReader reader(data.substr(2));

	bool result = false;
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
		default:
			this->connected = false;
	}

	if (!result)
	{
		puts("Action handler failed");
	}
}

void EOClient::SendBuilder(PacketBuilder &builder)
{
	std::string packet = static_cast<std::string>(builder);
	this->Send(this->processor.Encode(packet));
}
