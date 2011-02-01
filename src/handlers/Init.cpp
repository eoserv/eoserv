
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

#include <cmath>

#include "world.hpp"

static unsigned int stupid_hash(unsigned int i)
{
	++i;

	return 110905 + (i % 9 + 1) * ((11092004 - i) % ((i % 11 + 1) * 119)) * 119 + i % 2004;
}

CLIENT_F_FUNC(Init)
{
	if (this->state != EOClient::Uninitialized) return false;

	PacketBuilder reply;
	unsigned int challenge;
	unsigned int response;

	reply.SetID(0); // 0 is a special case which sends un-encrypted data

	challenge = reader.GetThree();

	reader.GetChar(); // ?
	reader.GetChar(); // ?
	this->version = reader.GetChar();
	reader.GetChar(); // ?
	reader.GetChar(); // ?

	try
	{
		this->hdid = static_cast<int>(util::to_uint_raw(reader.GetEndString()));
	}
	catch (std::invalid_argument)
	{
		this->Close();
		return false;
	}

	int ban_expires;
	IPAddress remote_addr = this->GetRemoteAddr();
	if ((ban_expires = this->server()->world->CheckBan(0, &remote_addr, &this->hdid)) != -1)
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
		CLIENT_SENDRAW(reply);
		this->Close();
		return false;
	}

	int minversion = this->server()->world->config["MinVersion"];
	if (!minversion)
	{
		minversion = 27;
	}

	int maxversion = this->server()->world->config["MaxVersion"];
	if (!maxversion)
	{
		maxversion = 28;
	}

	if (this->server()->world->config["CheckVersion"] && (this->version < minversion || this->version > maxversion))
	{
		reply.AddByte(INIT_OUT_OF_DATE);
		reply.AddChar(0);
		reply.AddChar(0);
		reply.AddChar(minversion);
		CLIENT_SENDRAW(reply);
		this->Close();
		return false;
	}

	response = stupid_hash(challenge);

	int emulti_e = util::rand(6,12);
	int emulti_d = util::rand(6,12);

	reply.AddByte(INIT_OK);
	reply.AddByte(10); // "eID" starting value (1 = +7)
	reply.AddByte(10); // "eID" starting value (1 = +1)
	reply.AddByte(emulti_e); // dickwinder multiple
	reply.AddByte(emulti_d); // dickwinder multiple
	reply.AddShort(this->id); // player id
	reply.AddThree(response); // hash result

	this->processor.SetEMulti(emulti_e, emulti_d);

	CLIENT_SENDRAW(reply);

	this->state = EOClient::Initialized;
	return true;
}
