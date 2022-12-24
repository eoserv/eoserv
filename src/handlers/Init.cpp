/* handlers/Init.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../config.hpp"
#include "../eoclient.hpp"
#include "../eoserver.hpp"
#include "../packet.hpp"
#include "../timer.hpp"
#include "../world.hpp"

#include "../console.hpp"
#include "../util.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <stdexcept>
#include <string>

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
	unsigned prot = reader.GetChar();
	unsigned hdid_len = reader.GetChar();
	std::string hdid_str = reader.GetEndString();

	try
	{
		client->hdid = int(util::to_uint_raw(hdid_str));
	}
	catch (std::invalid_argument&)
	{
		client->server()->RecordClientRejection(client->GetRemoteAddr(), "bad hdid");
		client->Close(true);
		return;
	}

	if (hdid_str.length() != hdid_len)
	{
		client->server()->RecordClientRejection(client->GetRemoteAddr(), "bad hdid");
		client->Close(true);
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
			char errbuf[64];
			std::snprintf(errbuf, sizeof errbuf, "too many connections from this PC: %08x", client->hdid);
			client->server()->RecordClientRejection(client->GetRemoteAddr(), errbuf);
			client->Close(true);
			return;
		}
	}

	int ban_expires;
	IPAddress remote_addr = client->GetRemoteAddr();
	if ((ban_expires = client->server()->world->CheckBan(0, &remote_addr, ignore_hdid ? 0 : &client->hdid)) != -1)
	{
		reply.AddByte(INIT_BANNED);
		if (ban_expires == 0)
		{
			client->server()->RecordClientRejection(client->GetRemoteAddr(), "perm ban");
			reply.AddByte(INIT_BAN_PERM);
		}
		else
		{
			client->server()->RecordClientRejection(client->GetRemoteAddr(), "temp ban");
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
		minversion = client->server()->world->config["OldVersionCompat"] ? 27 : 28;
	}

	int maxversion = client->server()->world->config["MaxVersion"];
	if (!maxversion)
	{
		maxversion = 28;
	}

	bool accepted_version = client->version >= minversion && (maxversion < 0 || client->version <= maxversion);

	if (prot != 112)
		accepted_version = false;

	if (client->server()->world->config["CheckVersion"] && !accepted_version)
	{
		client->server()->RecordClientRejection(client->GetRemoteAddr(), "out of date");
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

	client->InitNewSequence();
	auto seq_bytes = client->GetSeqInitBytes();

	reply.AddByte(INIT_OK);
	reply.AddByte(seq_bytes.first);
	reply.AddByte(seq_bytes.second);
	reply.AddByte(emulti_e);
	reply.AddByte(emulti_d);
	reply.AddShort(client->id);
	reply.AddThree(response);

	client->processor.SetEMulti(emulti_e, emulti_d);

	client->Send(reply);

	client->state = EOClient::Initialized;

	return;
}

PACKET_HANDLER_REGISTER(PACKET_F_INIT)
	Register(PACKET_A_INIT, Init_Init, Uninitialized);
PACKET_HANDLER_REGISTER_END(PACKET_F_INIT)

}
