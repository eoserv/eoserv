
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef PACKET_HPP_INCLUDED
#define PACKET_HPP_INCLUDED

#include <string>
#include <cstddef>

class PacketProcessor;
class PacketReader;
class PacketBuilder;

#include "util.hpp"

enum PacketFamily UTIL_EXTEND_ENUM(unsigned char)
{
	PACKET_CONNECTION = 1,
	PACKET_ACCOUNT = 2,
	PACKET_CHARACTER = 3,
	PACKET_LOGIN = 4,
	PACKET_WELCOME = 5,
	PACKET_WALK = 6,
	PACKET_FACE = 7,
	PACKET_CHAIR = 8,
	PACKET_EMOTE = 9,
	PACKET_ATTACK = 11,
	PACKET_SHOP = 13,
	PACKET_ITEM = 14,
	PACKET_STATSKILL = 16,
	PACKET_GLOBAL = 17,
	PACKET_TALK = 18,
	PACKET_WARP = 19,
	PACKET_JUKEBOX = 21,
	PACKET_PLAYERS = 22,
	PACKET_CLOTHES = 23,
	PACKET_PARTY = 24,
	PACKET_REFRESH = 25,
	PACKET_NPC = 26,
	PACKET_AUTOREFRESH = 27,
	PACKET_APPEAR = 29,
	PACKET_PAPERDOLL = 30,
	PACKET_EFFECT = 31,
	PACKET_TRADE = 32,
	PACKET_CHEST = 33,
	PACKET_DOOR = 34,
	PACKET_PING = 35,
	PACKET_BANK = 36,
	PACKET_LOCKER = 37,
	PACKET_GUILD = 39,
	PACKET_SIT = 41,
	PACKET_RECOVER = 42,
	PACKET_BOARD = 43,
	PACKET_ARENA = 45,
	PACKET_ADMININTERACT = 48,
	PACKET_CITIZEN = 49,
	PACKET_QUEST = 50,
	PACKET_BOOK = 51,
	PACKET_F_INIT = 255
};

enum PacketAction UTIL_EXTEND_ENUM(unsigned char)
{
	PACKET_REQUEST = 1,
	PACKET_ACCEPT = 2,
	PACKET_REPLY = 3,
	PACKET_REMOVE = 4,
	PACKET_AGREE = 5,
	PACKET_CREATE = 6,
	PACKET_ADD = 7,
	PACKET_PLAYER = 8,
	PACKET_TAKE = 9,
	PACKET_USE = 10,
	PACKET_BUY = 11,
	PACKET_SELL = 12,
	PACKET_OPEN = 13,
	PACKET_CLOSE = 14,
	PACKET_MSG = 15,
	PACKET_SPEC = 16,
	PACKET_ADMIN = 17,
	PACKET_LIST = 18,
	PACKET_TELL = 20,
	PACKET_REPORT = 21,
	PACKET_ANNOUNCE = 22,
	PACKET_SERVER = 23,
	PACKET_DROP = 24,
	PACKET_JUNK = 25,
	PACKET_GET = 27,
	PACKET_EXP = 33, // Tentative name
	PACKET_NET = 240,
	PACKET_NET2 = 241,
	PACKET_NET3 = 242,
	PACKET_A_INIT = 255
};

/**
 * Encodes and Decodes packets for a Client.
 * Each Client needs an instance of this because it holds connection-specific data required to function correctly.
 */
class PacketProcessor
{
	protected:
		/**
		 * "EMulti" variable for Encoding.
		 */
		unsigned char emulti_e;

		/**
		 * "EMulti" variable for Decoding.
		 */
		unsigned char emulti_d;

		/**
		 * Flag marking the first Decode call (which shouldn't be Decoded).
		 */
		bool firstdec;

	public:
		/**
		 * Highest number EO can represent with 1 byte.
		 */
		static const unsigned int MAX1 = 253;

		/**
		 * Highest number EO can represent with 2 bytes.
		 */
		static const unsigned int MAX2 = 64009;

		/**
		 * Highest number EO can represent with 3 bytes.
		 */
		static const unsigned int MAX3 = 16194277;

		PacketProcessor();

		/**
		 * Return a string describing a packet's family ID.
		 */
		static std::string GetFamilyName(PacketFamily family);

		/**
		 * Return a string describing a packet's action ID.
		 */
		static std::string GetActionName(PacketAction action);

		std::string Decode(const std::string &);
		std::string Encode(const std::string &);
		static std::string DickWinder(const std::string &, unsigned char emulti);
		std::string DickWinderE(const std::string &);
		std::string DickWinderD(const std::string &);

		void SetEMulti(unsigned char, unsigned char);

		static unsigned int Number(unsigned char, unsigned char = 254, unsigned char = 254, unsigned char = 254);
		static util::quadchar ENumber(unsigned int);
		static util::quadchar ENumber(unsigned int, std::size_t &size);

		static unsigned short PID(PacketFamily family, PacketAction action);
		static util::pairchar EPID(unsigned short id);
};

class PacketReader
{
	protected:
		std::string data;
		std::size_t length;

	public:
		PacketReader(const std::string &);

		std::size_t Length();
		std::size_t Remaining();

		unsigned char GetByte();
		unsigned char GetChar();
		unsigned short GetShort();
		unsigned int GetThree();
		unsigned int GetInt();

		std::string GetFixedString(std::size_t length);
		std::string GetBreakString(unsigned char breakchar = 0xFF);
		std::string GetEndString();
};

class PacketBuilder
{
	protected:
		unsigned short id;
		std::size_t length;
		std::string data;

	public:
		PacketBuilder();
		PacketBuilder(unsigned short id);
		PacketBuilder(PacketFamily family, PacketAction action);

		unsigned short SetID(unsigned short id);
		unsigned short SetID(PacketFamily family, PacketAction action);

		std::size_t Length();

		unsigned char AddByte(unsigned char);
		unsigned char AddChar(unsigned char);
		unsigned short AddShort(unsigned short);
		unsigned int AddThree(unsigned int);
		unsigned int AddInt(unsigned int);
		unsigned int AddVar(int min, int max, unsigned int);

		const std::string &AddString(const std::string &);
		const std::string &AddBreakString(const std::string &, unsigned char breakchar = 0xFF);

		void Reset();

		std::string Get();

		operator std::string();
};

#endif // PACKET_HPP_INCLUDED
