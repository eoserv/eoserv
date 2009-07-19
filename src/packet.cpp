
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "packet.hpp"

#include <string>
#include <algorithm>

#include "util.hpp"

PacketProcessor::PacketProcessor()
{
	this->firstdec = true;
}

std::string PacketProcessor::GetFamilyName(PacketFamily family)
{
	switch (family)
	{
		case PACKET_CONNECTION: return "Connection";
		case PACKET_ACCOUNT: return "Account";
		case PACKET_CHARACTER: return "Character";
		case PACKET_LOGIN: return "Login";
		case PACKET_WELCOME: return "Welcome";
		case PACKET_WALK: return "Walk";
		case PACKET_FACE: return "Face";
		case PACKET_CHAIR: return "Chair";
		case PACKET_EMOTE: return "Emote";
		case PACKET_ATTACK: return "Attack";
		case PACKET_SHOP: return "Shop";
		case PACKET_ITEM: return "Item";
		case PACKET_STATSKILL: return "StatSkill";
		case PACKET_GLOBAL: return "Global";
		case PACKET_TALK: return "Talk";
		case PACKET_WARP: return "Warp";
		case PACKET_JUKEBOX: return "Jukebox";
		case PACKET_PLAYERS: return "Players";
		case PACKET_CLOTHES: return "Character";
		case PACKET_PARTY: return "Party";
		case PACKET_REFRESH: return "Refresh";
		case PACKET_NPC: return "NPC";
		case PACKET_AUTOREFRESH: return "AutoRefresh";
		case PACKET_APPEAR: return "Appear";
		case PACKET_PAPERDOLL: return "Paperdoll";
		case PACKET_EFFECT: return "Effect";
		case PACKET_TRADE: return "Trade";
		case PACKET_CHEST: return "Chest";
		case PACKET_DOOR: return "Door";
		case PACKET_PING: return "Ping";
		case PACKET_BANK: return "Bank";
		case PACKET_LOCKER: return "Locker";
		case PACKET_GUILD: return "Guild";
		case PACKET_SIT: return "Sit";
		case PACKET_BOARD: return "Board";
		case PACKET_ARENA: return "Arena";
		case PACKET_ADMININTERACT: return "AdminInteract";
		case PACKET_CITIZEN: return "Citizen";
		case PACKET_QUEST: return "Quest";
		case PACKET_BOOK: return "Book";
		case PACKET_F_INIT: return "Init";
		default: return "UNKNOWN";
	}
}

std::string PacketProcessor::GetActionName(PacketAction action)
{
	switch (action)
	{
		case PACKET_REQUEST: return "Request";
		case PACKET_ACCEPT: return "Accept";
		case PACKET_REPLY: return "Reply";
		case PACKET_REMOVE: return "Remove";
		case PACKET_AGREE: return "Agree";
		case PACKET_CREATE: return "Create";
		case PACKET_ADD: return "Add";
		case PACKET_PLAYER: return "Player";
		case PACKET_TAKE: return "Take";
		case PACKET_USE: return "Use";
		case PACKET_BUY: return "Buy";
		case PACKET_SELL: return "Sell";
		case PACKET_OPEN: return "Open";
		case PACKET_CLOSE: return "Close";
		case PACKET_MSG: return "Msg";
		case PACKET_SPEC: return "Spec";
		case PACKET_ADMIN: return "Admin";
		case PACKET_LIST: return "List";
		case PACKET_TELL: return "Tell";
		case PACKET_REPORT: return "Report";
		case PACKET_ANNOUNCE: return "Announce";
		case PACKET_SERVER: return "Server";
		case PACKET_DROP: return "Drop";
		case PACKET_JUNK: return "Junk";
		case PACKET_GET: return "Get";
		case PACKET_NET: return "Net";
		case PACKET_NET2: return "Net2";
		case PACKET_NET3: return "Net3";
		case PACKET_A_INIT: return "Init";
		default: return "UNKNOWN";
	}
}

std::string PacketProcessor::Decode(const std::string &str)
{
	std::string newstr;
	int length = str.length();
	int i = 0;
	int ii = 0;

	if (this->firstdec)
	{
		this->firstdec = false;
		return str;
	}

	newstr.resize(length);

	while (i < length)
	{
		newstr[ii++] = (unsigned char)str[i] ^ 0x80;
		i += 2;
	}

	--i;

	if (length % 2)
	{
		i -= 2;
	}

	do
	{
		newstr[ii++] = (unsigned char)str[i] ^ 0x80;
		i -= 2;
	} while (i >= 0);

	for (int i = 2; i < length; ++i)
	{
		if (static_cast<unsigned char>(newstr[i]) == 128)
		{
			newstr[i] = 0;
		}
		else if (newstr[i] == 0)
		{
			newstr[i] = 128;
		}
	}

	return this->DickWinderD(newstr);
}

std::string PacketProcessor::Encode(const std::string &rawstr)
{
	std::string str = this->DickWinderE(rawstr);
	std::string newstr;
	int length = str.length();
	int i = 2;
	int ii = 2;

	newstr.resize(length);

	newstr[0] = str[0];
	newstr[1] = str[1];

	while (i < length)
	{
		newstr[i] = (unsigned char)str[ii++] ^ 0x80;
		i += 2;
	}

	i = length - 1;

	if (length % 2)
	{
		--i;
	}

	while (i >= 2)
	{
		newstr[i] = (unsigned char)str[ii++] ^ 0x80;
		i -= 2;
	}

	for (int i = 2; i < length; ++i)
	{
		if (static_cast<unsigned char>(newstr[i]) == 128)
		{
			newstr[i] = 0;
		}
		else if (newstr[i] == 0)
		{
			newstr[i] = 128;
		}
	}

	return newstr;
}

std::string PacketProcessor::DickWinder(const std::string &str, unsigned char emulti)
{
	std::string newstr;
	int length = str.length();
	std::string buffer;
	unsigned char c;

	if (emulti == 0)
	{
		return str;
	}

	for (int i = 0; i < length; ++i)
	{
		c = str[i];

		if (c % emulti == 0)
		{
			buffer += c;
		}
		else
		{
			if (buffer.length() > 0)
			{
				std::reverse(buffer.begin(), buffer.end());
				newstr += buffer;
				buffer.clear();
			}
			newstr += c;
		}
	}

	if (buffer.length() > 0)
	{
		std::reverse(buffer.begin(), buffer.end());
		newstr += buffer;
	}

	return newstr;
}

std::string PacketProcessor::DickWinderE(const std::string &str)
{
	return PacketProcessor::DickWinder(str, this->emulti_e);
}

std::string PacketProcessor::DickWinderD(const std::string &str)
{
	return PacketProcessor::DickWinder(str, this->emulti_d);
}

void PacketProcessor::SetEMulti(unsigned char emulti_e, unsigned char emulti_d)
{
	this->emulti_e = emulti_e;
	this->emulti_d = emulti_d;
}

unsigned int PacketProcessor::Number(unsigned char b1, unsigned char b2, unsigned char b3, unsigned char b4)
{
	if (b1 == 0 || b1 == 254) b1 = 1;
	if (b2 == 0 || b2 == 254) b2 = 1;
	if (b3 == 0 || b3 == 254) b3 = 1;
	if (b4 == 0 || b4 == 254) b4 = 1;

	--b1;
	--b2;
	--b3;
	--b4;

	return (b4*PacketProcessor::MAX3 + b3*PacketProcessor::MAX2 + b2*PacketProcessor::MAX1 + b1);
}

util::quadchar PacketProcessor::ENumber(unsigned int number)
{
	std::size_t throwaway;

	return PacketProcessor::ENumber(number, throwaway);
}

util::quadchar PacketProcessor::ENumber(unsigned int number, std::size_t &size)
{
	util::quadchar bytes(254);
	unsigned int onumber = number;

	if (onumber >= PacketProcessor::MAX3)
	{
		bytes[3] = number / PacketProcessor::MAX3 + 1;
		number = number % PacketProcessor::MAX3;
	}

	if (onumber >= PacketProcessor::MAX2)
	{
		bytes[2] = number / PacketProcessor::MAX2 + 1;
		number = number % PacketProcessor::MAX2;
	}

	if (onumber >= PacketProcessor::MAX1)
	{
		bytes[1] = number / PacketProcessor::MAX1 + 1;
		number = number % PacketProcessor::MAX1;
	}

	bytes[0] = number + 1;

	for (int i = 3; i >= 0; --i)
	{
		if (i == 0)
		{
			size = 1;
			break;
		}
		else if (bytes[i] > 0)
		{
			size = i + 1;
			break;
		}
	}

	return bytes;
}

unsigned short PacketProcessor::PID(PacketFamily b1, PacketAction b2)
{
	return b1 + b2*256;
}

util::pairchar PacketProcessor::EPID(unsigned short pid)
{
	util::pairchar bytes;

	bytes[1] = pid % 256;
	bytes[0] = pid / 256;

	return bytes;
}

PacketReader::PacketReader(const std::string &data)
{
	this->data = data;
	this->length = data.length();
}

std::size_t PacketReader::Length()
{
	return this->length;
}

std::size_t PacketReader::Remaining()
{
	return this->data.length();
}

unsigned char PacketReader::GetByte()
{
	unsigned char ret;

	if (this->data.length() < 1)
	{
		return 0;
	}

	ret = this->data[0];
	this->data.erase(0, 1);

	return ret;
}

unsigned char PacketReader::GetChar()
{
	unsigned char ret;

	if (this->data.length() < 1)
	{
		return 0;
	}

	ret = PacketProcessor::Number(this->data[0]);
	this->data.erase(0, 1);

	return ret;
}

unsigned short PacketReader::GetShort()
{
	unsigned short ret;

	if (this->data.length() < 1)
	{
		return 0;
	}

	if (this->data.length() < 2)
	{
		ret = PacketProcessor::Number(this->data[0]);
		this->data.erase(0, 1);
		return ret;
	}

	ret = PacketProcessor::Number(this->data[0], this->data[1]);
	this->data.erase(0, 2);

	return ret;
}

unsigned int PacketReader::GetThree()
{
	unsigned int ret;

	if (this->data.length() < 1)
	{
		return 0;
	}

	if (this->data.length() < 2)
	{
		ret = PacketProcessor::Number(this->data[0]);
		this->data.erase(0, 1);
		return ret;
	}

	if (this->data.length() < 3)
	{
		ret = PacketProcessor::Number(this->data[0], this->data[1]);
		this->data.erase(0, 2);
		return ret;
	}

	ret = PacketProcessor::Number(this->data[0], this->data[1], this->data[2]);
	this->data.erase(0, 3);

	return ret;
}

unsigned int PacketReader::GetInt()
{
	unsigned int ret;

	if (this->data.length() < 1)
	{
		return 0;
	}

	if (this->data.length() < 2)
	{
		ret = PacketProcessor::Number(this->data[0]);
		this->data.erase(0, 1);
		return ret;
	}

	if (this->data.length() < 3)
	{
		ret = PacketProcessor::Number(this->data[0], this->data[1]);
		this->data.erase(0, 2);
		return ret;
	}

	if (this->data.length() < 4)
	{
		ret = PacketProcessor::Number(this->data[0], this->data[1], this->data[2]);
		this->data.erase(0, 3);
		return ret;
	}

	ret = PacketProcessor::Number(this->data[0], this->data[1], this->data[2], this->data[3]);
	this->data.erase(0, 4);

	return ret;
}

std::string PacketReader::GetFixedString(std::size_t length)
{
	std::string ret;

	if (length == 0 || this->data.length() < length)
	{
		return ret;
	}

	ret = this->data.substr(0, length);
	this->data.erase(0, length);

	return ret;
}

std::string PacketReader::GetBreakString(unsigned char breakchar)
{
	std::string ret;
	std::size_t length;

	length = this->data.find_first_of(breakchar);

	if (length == std::string::npos)
	{
		return ret;
	}

	ret = this->data.substr(0, length);
	this->data.erase(0, length+1);

	return ret;
}

std::string PacketReader::GetEndString()
{
	std::string ret = this->data;

	this->data.erase();

	return ret;
}

PacketBuilder::PacketBuilder()
{
	this->length = 0;
	this->id = 0;
}

PacketBuilder::PacketBuilder(unsigned short id)
{
	this->length = 0;
	this->SetID(id);
}

PacketBuilder::PacketBuilder(PacketFamily family, PacketAction action)
{
	this->length = 0;
	this->SetID(family, action);
}

unsigned short PacketBuilder::SetID(unsigned short id)
{
	if (id == 0)
	{
		id = PacketProcessor::PID(PACKET_F_INIT, PACKET_A_INIT);
	}

	this->id = id;

	return this->id;
}

unsigned short PacketBuilder::SetID(PacketFamily family, PacketAction action)
{
	return this->SetID(PacketProcessor::PID(family,action));
}

unsigned char PacketBuilder::AddByte(unsigned char byte)
{
	++this->length;
	this->data += byte;
	return byte;
}

unsigned char PacketBuilder::AddChar(unsigned char num)
{
	util::quadchar bytes;
	++this->length;
	bytes = PacketProcessor::ENumber(num);
	this->data += bytes[0];
	return num;
}

unsigned short PacketBuilder::AddShort(unsigned short num)
{
	util::quadchar bytes;
	this->length += 2;
	bytes = PacketProcessor::ENumber(num);
	this->data += bytes[0];
	this->data += bytes[1];
	return num;
}

unsigned int PacketBuilder::AddThree(unsigned int num)
{
	util::quadchar bytes;
	this->length += 3;
	bytes = PacketProcessor::ENumber(num);
	this->data += bytes[0];
	this->data += bytes[1];
	this->data += bytes[2];
	return num;
}

unsigned int PacketBuilder::AddInt(unsigned int num)
{
	util::quadchar bytes;
	this->length += 4;
	bytes = PacketProcessor::ENumber(num);
	this->data += bytes[0];
	this->data += bytes[1];
	this->data += bytes[2];
	this->data += bytes[3];
	return num;
}

unsigned int PacketBuilder::AddVar(int min, int max, unsigned int num)
{
	if (min <= 1)
	{
		if (max <= 1 || num < PacketProcessor::MAX1)
		{
			this->AddChar(num);
			return num;
		}
	}

	if (min <= 2)
	{
		if (max <= 2 || num < PacketProcessor::MAX2)
		{
			this->AddShort(num);
			return num;
		}
	}

	if (min <= 3)
	{
		if (max <= 3 || num < PacketProcessor::MAX3)
		{
			this->AddThree(num);
			return num;
		}
	}

	this->AddInt(num);
	return num;
}

const std::string &PacketBuilder::AddString(const std::string &str)
{
	this->length += str.length();
	this->data += str;

	return str;
}

const std::string &PacketBuilder::AddBreakString(const std::string &str, unsigned char breakchar)
{
	std::string tempstr(str);
	std::size_t breakin = tempstr.find_first_of(breakchar);
	while (breakin != std::string::npos)
	{
		tempstr[breakin] = 'y';
		breakin = tempstr.find_first_of(breakchar, breakin+1);
	}

	this->length += tempstr.length() + 1;
	this->data += tempstr;
	this->data += breakchar;

	return str;
}

void PacketBuilder::Reset()
{
	this->length = 0;
	this->data.erase();
}

std::string PacketBuilder::Get()
{
	std::string retdata;
	util::pairchar id = PacketProcessor::EPID(this->id);
	util::quadchar length = PacketProcessor::ENumber(this->length + 2);

	retdata += length[0];
	retdata += length[1];
	retdata += id[0];
	retdata += id[1];
	retdata += this->data;

	return retdata;
}

PacketBuilder::operator std::string()
{
	return this->Get();
}
