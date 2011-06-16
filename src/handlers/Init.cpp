
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include <cmath>
#include <stdexcept>

#include "console.hpp"
#include "eoclient.hpp"
#include "world.hpp"

static inline unsigned int stupid_hash(unsigned int i)
{
	++i;

	return 110905 + (i % 9 + 1) * ((11092004 - i) % ((i % 11 + 1) * 119)) * 119 + i % 2004;
}

namespace Handlers
{

void Init_Init(EOClient *client, PacketReader &reader)
{
	PacketBuilder reply(PACKET_F_INIT, PACKET_A_INIT, 10);

	unsigned int challenge;
	unsigned int response;

	challenge = reader.GetThree();

	reader.GetChar(); // ?
	reader.GetChar(); // ?
	client->version = reader.GetChar();
	reader.GetChar(); // ?
	reader.GetChar(); // ?

	try
	{
		client->hdid = int(util::to_uint_raw(reader.GetEndString()));
	}
	catch (std::invalid_argument)
	{
		client->Close();
		return;
	}

	int pc_connections = 0;

	UTIL_FOREACH(client->server()->clients, checkclient)
	{
		EOClient *checkeoclient = static_cast<EOClient *>(checkclient);

		if (checkeoclient->hdid == client->hdid && checkeoclient->GetRemoteAddr() == client->GetRemoteAddr())
		{
			++pc_connections;
		}
	}
	
	const bool ignore_hdid = client->server()->world->config["IgnoreHDID"];

	if (!ignore_hdid)
	{
		const int max_per_pc = int(client->server()->world->config["MaxConnectionsPerPC"]);

		if (max_per_pc != 0 && pc_connections > max_per_pc)
		{
			Console::Wrn("Connection from %s was rejected (too many connections from this PC: %08x)", static_cast<std::string>(client->GetRemoteAddr()).c_str(), client->hdid);
			client->Close(true);
		}
	}

	int ban_expires;
	IPAddress remote_addr = client->GetRemoteAddr();
	if ((ban_expires = client->server()->world->CheckBan(0, &remote_addr, ignore_hdid ? 0 : &client->hdid)) != -1)
	{
		reply.AddByte(INIT_BANNED);
		if (ban_expires == 0)
		{
			reply.AddByte(INIT_BAN_PERM);
		}
		else
		{
			int mins_remaining = int(std::min(255.0, std::ceil(double(ban_expires - std::time(0)) / 60.0)));
			reply.AddByte(INIT_BAN_TEMP);
			reply.AddByte(mins_remaining);
		}
		client->Send(reply);
		client->Close();
		return;
	}

	int minversion = client->server()->world->config["MinVersion"];
	if (!minversion)
	{
		minversion = client->server()->world->config["Version27Compat"] ? 27 : 28;
	}

	int maxversion = client->server()->world->config["MaxVersion"];
	if (!maxversion)
	{
		maxversion = 28;
	}

	if (client->server()->world->config["OldVersionCompat"] && (client->version < minversion || client->version > maxversion))
	{
		reply.AddByte(INIT_OUT_OF_DATE);
		reply.AddChar(0);
		reply.AddChar(0);
		reply.AddChar(minversion);
		client->Send(reply);
		client->Close();
		return;
	}

	response = stupid_hash(challenge);

	int emulti_e = util::rand(6,12);
	int emulti_d = util::rand(6,12);

	reply.AddByte(INIT_OK);
	reply.AddByte(10); // "eID" starting value (1 = +7)
	reply.AddByte(10); // "eID" starting value (1 = +1)
	reply.AddByte(emulti_e); // dickwinder multiple
	reply.AddByte(emulti_d); // dickwinder multiple
	reply.AddShort(client->id); // player id
	reply.AddThree(response); // hash result

	client->processor.SetEMulti(emulti_e, emulti_d);

	client->Send(reply);

	client->state = EOClient::Initialized;

	return;
}

PACKET_HANDLER_REGISTER(PACKET_F_INIT)
	Register(PACKET_A_INIT, Init_Init, Uninitialized);
PACKET_HANDLER_REGISTER_END()

}
