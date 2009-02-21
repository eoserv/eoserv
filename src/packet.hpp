#ifndef PACKET_HPP_INCLUDED
#define PACKET_HPP_INCLUDED

#include <string>
#include <stdint.h>
#include <cstddef>

#include "util.hpp"

class PacketProcessor;
class PacketReader;
class PacketBuilder;

// FAMILY
const uint8_t PACKET_CONNECTION = 1;
const uint8_t PACKET_ACCOUNT = 2;
const uint8_t PACKET_CHARACTER = 3;
const uint8_t PACKET_LOGIN = 4;
const uint8_t PACKET_WELCOME = 5;
const uint8_t PACKET_WALK = 6;
const uint8_t PACKET_FACE = 7;
const uint8_t PACKET_CHAIR = 8;
const uint8_t PACKET_EMOTE = 9;
const uint8_t PACKET_ATTACK = 11;
const uint8_t PACKET_SHOP = 13;
const uint8_t PACKET_ITEM = 14;
const uint8_t PACKET_SKILLMASTER = 16;
const uint8_t PACKET_GLOBAL = 17;
const uint8_t PACKET_TALK = 18;
const uint8_t PACKET_WARP = 19;
const uint8_t PACKET_JUKEBOX = 21;
const uint8_t PACKET_PLAYERS = 22;
const uint8_t PACKET_PARTY = 24;
const uint8_t PACKET_REFRESH = 25;
const uint8_t PACKET_PAPERDOLL = 30;
const uint8_t PACKET_TRADE = 32;
const uint8_t PACKET_CHEST = 33;
const uint8_t PACKET_DOOR = 34;
const uint8_t PACKET_PING = 35;
const uint8_t PACKET_BANK = 36;
const uint8_t PACKET_LOCKER = 37;
const uint8_t PACKET_GUILD = 39;
const uint8_t PACKET_SIT = 41;
const uint8_t PACKET_BOARD = 43;
const uint8_t PACKET_ARENA = 45;
const uint8_t PACKET_ADMININTERACT = 48;
const uint8_t PACKET_CITIZEN = 49;
const uint8_t PACKET_QUEST = 50;
const uint8_t PACKET_BOOK = 51;
const uint8_t PACKET_INIT = 255; // Also action

// ACTION
const uint8_t PACKET_REQUEST = 1;
const uint8_t PACKET_ACCEPT = 2;
const uint8_t PACKET_REPLY = 3;
const uint8_t PACKET_REMOVE = 4;
const uint8_t PACKET_AGREE = 5;
const uint8_t PACKET_CREATE = 6;
const uint8_t PACKET_ADD = 7;
const uint8_t PACKET_PLAYER = 8;
const uint8_t PACKET_TAKE = 9;
const uint8_t PACKET_USE = 10;
const uint8_t PACKET_BUY = 11;
const uint8_t PACKET_SELL = 12;
const uint8_t PACKET_OPEN = 13;
const uint8_t PACKET_CLOSE = 14;
const uint8_t PACKET_MSG = 15;
const uint8_t PACKET_MOVESPEC = 16;
const uint8_t PACKET_LIST = 18;
const uint8_t PACKET_TELL = 20;
const uint8_t PACKET_REPORT = 21;
const uint8_t PACKET_DROP = 24;
const uint8_t PACKET_JUNK = 25;
const uint8_t PACKET_GET = 27;
const uint8_t PACKET_NET = 240;

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
		uint32_t emulti_e;

		/**
		 * "EMulti" variable for Decoding.
		 */
		uint32_t emulti_d;

		/**
		 * Flag marking the first Decode call (which shouldn't be Decoded).
		 */
		bool firstdec;

	public:
		/**
		 * Highest number EO can represent with 1 byte.
		 */
		static const uint32_t MAX1 = 253;

		/**
		 * Highest number EO can represent with 2 bytes.
		 */
		static const uint32_t MAX2 = 64009;

		/**
		 * Highest number EO can represent with 3 bytes.
		 */
		static const uint32_t MAX3 = 16194277;

		PacketProcessor();

		/**
		 * Return a string describing a packet's family ID.
		 */
		static std::string GetFamilyName(uint8_t family);

		/**
		 * Return a string describing a packet's action ID.
		 */
		static std::string GetActionName(uint8_t action);

		std::string Decode(const std::string &);
		std::string Encode(const std::string &);
		std::string DickWinder(const std::string &, uint8_t emulti);
		std::string DickWinderE(const std::string &);
		std::string DickWinderD(const std::string &);

		void SetEMulti(uint8_t, uint8_t);

		static uint32_t Number(uint8_t, uint8_t = 254, uint8_t = 254, uint8_t = 254);
		static quadchar ENumber(uint32_t);
		static quadchar ENumber(uint32_t, std::size_t &size);

		static uint16_t PID(uint8_t family, uint8_t action);
		static pairchar EPID(uint16_t id);
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

		uint8_t GetByte();
		uint8_t GetChar();
		uint16_t GetShort();
		uint32_t GetThree();
		uint32_t GetInt();

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
		PacketBuilder(uint16_t id);
		PacketBuilder(uint8_t family, uint8_t action);

		uint16_t SetID(uint16_t id);
		uint16_t SetID(uint8_t family, uint8_t action);

		std::size_t Length();

		uint8_t AddByte(uint8_t);
		uint8_t AddChar(uint8_t);
		uint16_t AddShort(uint16_t);
		uint32_t AddThree(uint32_t);
		uint32_t AddInt(uint32_t);

		const std::string &AddString(const std::string &);
		const std::string &AddBreakString(const std::string &, unsigned char breakchar = 0xFF);

		std::string Get();

		operator std::string();
};

#endif // PACKET_HPP_INCLUDED
