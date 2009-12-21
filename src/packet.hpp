
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef PACKET_HPP_INCLUDED
#define PACKET_HPP_INCLUDED

#include "stdafx.h"

enum PacketFamily UTIL_EXTEND_ENUM(unsigned char)
{
	PACKET_INTERNAL = 0,
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
	PACKET_CLOTHES = 23, // Should really be called something like PACKET_CHARACTER
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

	PACKET_INTERNAL_NULL = 128,
	PACKET_INTERNAL_WARP = 129,

	PACKET_NET = 240,
	PACKET_NET2 = 241,
	PACKET_NET3 = 242,
	PACKET_A_INIT = 255
};

/**
 * Encodes and Decodes packets for a Client.
 * Each Client needs an instance of this because it holds connection-specific data required to function correctly.
 */
class PacketProcessor : public Shared
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

	SCRIPT_REGISTER_REF_DF(PacketProcessor)
		SCRIPT_REGISTER_FUNCTION("string Decode(string &in)", Decode);
		SCRIPT_REGISTER_FUNCTION("string Encode(string &in)", Encode);
		SCRIPT_REGISTER_FUNCTION("string DickWinderE(string &in)", Encode);
		SCRIPT_REGISTER_FUNCTION("string DickWinderD(string &in)", Encode);
		SCRIPT_REGISTER_FUNCTION("void SetEMulti(uint8, uint8)", SetEMulti);

		SCRIPT_REGISTER_GLOBAL_FUNCTION("string PacketProcessor_DickWinder(string &in, uint8 emulti)", DickWinder);
		SCRIPT_REGISTER_GLOBAL_FUNCTION("string PacketProcessor_Number(uint8, uint8, uint8, uint8)", Number);
		// ENumber
		SCRIPT_REGISTER_GLOBAL_FUNCTION("string PacketProcessor_PID(PacketFamily family, PacketAction action)", PID);
		// EPID
	SCRIPT_REGISTER_END()
};

class PacketReader : public Shared
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

	static PacketReader *ScriptFactory(const std::string &str) { return new PacketReader(str); }

	SCRIPT_REGISTER_REF(PacketReader)
		SCRIPT_REGISTER_FACTORY("PacketReader @f(string &in)", ScriptFactory);

		SCRIPT_REGISTER_FUNCTION("uint Length()", Length);
		SCRIPT_REGISTER_FUNCTION("uint Remaining()", Remaining);
		SCRIPT_REGISTER_FUNCTION("uint8 GetByte()", GetByte);
		SCRIPT_REGISTER_FUNCTION("uint8 GetChar()", GetChar);
		SCRIPT_REGISTER_FUNCTION("uint16 GetShort()", GetShort);
		SCRIPT_REGISTER_FUNCTION("uint GetThree()", GetThree);
		SCRIPT_REGISTER_FUNCTION("uint GetInt()", GetInt);
		SCRIPT_REGISTER_FUNCTION("string GetFixedString(uint)", GetFixedString);
		SCRIPT_REGISTER_FUNCTION("string GetBreakString(uint8 breakchar)", GetBreakString);
		SCRIPT_REGISTER_FUNCTION("string GetEndString()", GetEndString);
	SCRIPT_REGISTER_END()
};

class PacketBuilder : public Shared
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

		std::string Get() const;

		operator std::string() const;

	static PacketBuilder *ScriptRegisterID(unsigned short id) { return new PacketBuilder(id); }
	static PacketBuilder *ScriptRegisterID2(PacketFamily family, PacketAction action) { return new PacketBuilder(family, action); }

	SCRIPT_REGISTER_REF_DF(PacketBuilder)
		SCRIPT_REGISTER_FACTORY("PacketBuilder @f(uint16 id)", ScriptRegisterID);
		SCRIPT_REGISTER_FACTORY("PacketBuilder @f(PacketFamily family, PacketAction action)", ScriptRegisterID2);

		SCRIPT_REGISTER_FUNCTION_PR("uint16 SetID(uint16 id)", SetID, (unsigned short), unsigned short);
		SCRIPT_REGISTER_FUNCTION_PR("uint16 SetID(PacketFamily family, PacketAction action)", SetID, (PacketFamily, PacketAction), unsigned short);
		SCRIPT_REGISTER_FUNCTION("uint Length()", Length);
		SCRIPT_REGISTER_FUNCTION("uint8 AddByte(uint8)", AddByte);
		SCRIPT_REGISTER_FUNCTION("uint8 AddChar(uint8)", AddChar);
		SCRIPT_REGISTER_FUNCTION("uint16 AddShort(uint16)", AddShort);
		SCRIPT_REGISTER_FUNCTION("uint AddThree(uint)", AddThree);
		SCRIPT_REGISTER_FUNCTION("uint AddInt(uint)", AddInt);
		SCRIPT_REGISTER_FUNCTION("uint AddVar(int min, int max, uint)", AddVar);
		SCRIPT_REGISTER_FUNCTION("string &AddString(string &in)", AddString);
		SCRIPT_REGISTER_FUNCTION("string &AddBreakString(string &in, uint8 breakchar)",AddBreakString);
		SCRIPT_REGISTER_FUNCTION("void Reset()", Reset);
		SCRIPT_REGISTER_FUNCTION("string Get()", Get);
	SCRIPT_REGISTER_END()
};

namespace packet
{

inline void ScriptRegister(ScriptEngine &engine)
{
	SCRIPT_REGISTER_ENUM("PacketFamily")
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_INTERNAL);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_CONNECTION);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_ACCOUNT);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_CHARACTER);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_LOGIN);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_WELCOME);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_WALK);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_FACE);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_CHAIR);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_EMOTE);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_ATTACK);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_SHOP);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_ITEM);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_STATSKILL);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_GLOBAL);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_TALK);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_WARP);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_JUKEBOX);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_PLAYERS);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_CLOTHES);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_PARTY);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_REFRESH);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_NPC);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_AUTOREFRESH);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_APPEAR);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_PAPERDOLL);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_EFFECT);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_TRADE);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_CHEST);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_DOOR);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_PING);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_BANK);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_LOCKER);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_GUILD);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_SIT);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_RECOVER);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_BOARD);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_ARENA);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_ADMININTERACT);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_CITIZEN);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_QUEST);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_BOOK);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_F_INIT);
	SCRIPT_REGISTER_ENUM_END()

	SCRIPT_REGISTER_ENUM("PacketAction")
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_REQUEST);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_ACCEPT);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_REPLY);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_REMOVE);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_AGREE);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_CREATE);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_ADD);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_PLAYER);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_TAKE);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_USE);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_BUY);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_SELL);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_OPEN);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_CLOSE);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_MSG);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_SPEC);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_ADMIN);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_LIST);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_TELL);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_REPORT);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_ANNOUNCE);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_SERVER);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_DROP);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_JUNK);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_GET);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_EXP);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_INTERNAL_NULL);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_INTERNAL_WARP);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_NET);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_NET2);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_NET3);
		SCRIPT_REGISTER_ENUM_VALUE(PACKET_A_INIT);
	SCRIPT_REGISTER_ENUM_END()
}

};

#endif // PACKET_HPP_INCLUDED
