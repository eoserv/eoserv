
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include <cmath>
#include <stdexcept>

#include "eoclient.hpp"
#include "world.hpp"

static inline unsigned int stupid_hash(unsigned int i)
{
	++i;

	return 110905 + (i % 9 + 1) * ((11092004 - i) % ((i % 11 + 1) * 119)) * 119 + i % 2004;
}

namespace Handlers
{

// Check if a character exists
void Init_Init(EOClient *client, PacketReader &reader)
{
	PacketBuilder reply;

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
		client->hdid = static_cast<int>(util::to_uint_raw(reader.GetEndString()));
	}
	catch (std::invalid_argument)
	{
		client->Close();
		return;
	}

	int ban_expires;
	IPAddress remote_addr = client->GetRemoteAddr();
	if ((ban_expires = client->server()->world->CheckBan(0, &remote_addr, &client->hdid)) != -1)
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
		minversion = 27;
	}

	int maxversion = client->server()->world->config["MaxVersion"];
	if (!maxversion)
	{
		maxversion = 28;
	}

	if (client->server()->world->config["CheckVersion"] && (client->version < minversion || client->version > maxversion))
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
