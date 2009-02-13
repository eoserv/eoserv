#ifndef PACKET_HPP_INCLUDED
#define PACKET_HPP_INCLUDED

#include <string>

#include "util.hpp"

class PacketProcessor;
class PacketReader;
class PacketBuilder;

// FAMILY
const unsigned char PACKET_CONNECTION = 1;
const unsigned char PACKET_ACCOUNT = 2;
const unsigned char PACKET_CHARACTER = 3;
const unsigned char PACKET_LOGIN = 4;
const unsigned char PACKET_WELCOME = 5;
const unsigned char PACKET_WALK = 6;
const unsigned char PACKET_FACE = 7;
const unsigned char PACKET_CHAIR = 8;
const unsigned char PACKET_EMOTE = 9;
const unsigned char PACKET_ATTACK = 11;
const unsigned char PACKET_SHOP = 13;
const unsigned char PACKET_ITEM = 14;
const unsigned char PACKET_SKILLMASTER = 16;
const unsigned char PACKET_GLOBAL = 17;
const unsigned char PACKET_TALK = 18;
const unsigned char PACKET_WARP = 19;
const unsigned char PACKET_JUKEBOX = 21;
const unsigned char PACKET_PLAYERS = 22;
const unsigned char PACKET_PARTY = 24;
const unsigned char PACKET_REFRESH = 25;
const unsigned char PACKET_PAPERDOLL = 30;
const unsigned char PACKET_TRADE = 32;
const unsigned char PACKET_CHEST = 33;
const unsigned char PACKET_DOOR = 34;
const unsigned char PACKET_PING = 35;
const unsigned char PACKET_BANK = 36;
const unsigned char PACKET_LOCKER = 37;
const unsigned char PACKET_GUILD = 39;
const unsigned char PACKET_SIT = 41;
const unsigned char PACKET_BOARD = 43;
const unsigned char PACKET_ARENA = 45;
const unsigned char PACKET_ADMININTERACT = 48;
const unsigned char PACKET_CITIZEN = 49;
const unsigned char PACKET_QUEST = 50;
const unsigned char PACKET_BOOK = 51;
const unsigned char PACKET_INIT = 255; // Also action

// ACTION
const unsigned char PACKET_REQUEST = 1;
const unsigned char PACKET_ACCEPT = 2;
const unsigned char PACKET_REPLY = 3;
const unsigned char PACKET_REMOVE = 4;
const unsigned char PACKET_AGREE = 5;
const unsigned char PACKET_CREATE = 6;
const unsigned char PACKET_ADD = 7;
const unsigned char PACKET_PLAYER = 8;
const unsigned char PACKET_TAKE = 9;
const unsigned char PACKET_USE = 10;
const unsigned char PACKET_BUY = 11;
const unsigned char PACKET_SELL = 12;
const unsigned char PACKET_OPEN = 13;
const unsigned char PACKET_CLOSE = 14;
const unsigned char PACKET_MSG = 15;
const unsigned char PACKET_MOVESPEC = 16;
const unsigned char PACKET_LIST = 18;
const unsigned char PACKET_TELL = 20;
const unsigned char PACKET_REPORT = 21;
const unsigned char PACKET_DROP = 24;
const unsigned char PACKET_JUNK = 25;
const unsigned char PACKET_GET = 27;
const unsigned char PACKET_NET = 240;

class PacketProcessor
{
	protected:
		int emulti_e;
		int emulti_d;
		bool firstdec;

	public:
		static const unsigned int MAX1 = 253;
		static const unsigned int MAX2 = 64009;
		static const unsigned int MAX3 = 16194277;

		PacketProcessor();

		static std::string GetFamilyName(unsigned char family);
		static std::string GetActionName(unsigned char action);

		std::string Decode(const std::string &);
		std::string Encode(const std::string &);
		std::string DickWinder(const std::string &, unsigned char emulti);
		std::string DickWinderE(const std::string &);
		std::string DickWinderD(const std::string &);

		void SetEMulti(unsigned char, unsigned char);

		static int Number(unsigned char, unsigned char = 254, unsigned char = 254, unsigned char = 254);
		static quadchar ENumber(unsigned int);
		static quadchar ENumber(unsigned int, int &size);

		static unsigned short PID(unsigned char family, unsigned char action);
		static pairchar EPID(unsigned short id);
};

class PacketReader
{
	protected:
		std::string data;
		size_t length;

	public:
		PacketReader(const std::string &);

		size_t Length();
		size_t Remaining();

		unsigned char GetByte();
		unsigned char GetChar();
		unsigned short GetShort();
		unsigned int GetThree();
		unsigned int GetInt();

		std::string GetFixedString(size_t length);
		std::string GetBreakString(unsigned char breakchar = 0xFF);
		std::string GetEndString();
};

class PacketBuilder
{
	protected:
		unsigned short id;
		size_t length;
		std::string data;

	public:
		PacketBuilder();
		PacketBuilder(unsigned short id);
		PacketBuilder(unsigned char family, unsigned char action);

		unsigned short SetID(unsigned short id);
		unsigned short SetID(unsigned char family, unsigned char action);

		size_t Length();

		unsigned char AddByte(unsigned char);
		unsigned char AddChar(unsigned char);
		unsigned short AddShort(unsigned short);
		unsigned int AddThree(unsigned int);
		unsigned int AddInt(unsigned int);

		const std::string &AddString(const std::string &);
		const std::string &AddBreakString(const std::string &, unsigned char breakchar = 0xFF);

		std::string Get();

		operator std::string();
};

#endif // PACKET_HPP_INCLUDED
