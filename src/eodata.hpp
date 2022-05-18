/* eodata.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef EODATA_HPP_INCLUDED
#define EODATA_HPP_INCLUDED

#include "fwd/eodata.hpp"

#include <array>
#include <string>
#include <vector>

struct Pub_File
{
	std::string filename;
	std::vector<std::size_t> splits;
};

/**
 * One item record in an EIF object
 */
template <class EIF> struct EIF_Data_Base
{
	int id;
	std::string name;
	short graphic;
	typename EIF::Type type;
	typename EIF::SubType subtype;

	typename EIF::Special special;
	short hp;
	short tp;
	short mindam;
	short maxdam;
	short accuracy;
	short evade;
	short armor;

	unsigned char str;
	unsigned char intl;
	unsigned char wis;
	unsigned char agi;
	unsigned char con;
	unsigned char cha;

	unsigned char light;
	unsigned char dark;
	unsigned char earth;
	unsigned char air;
	unsigned char water;
	unsigned char fire;

	union
	{
		int scrollmap;
		int dollgraphic;
		int expreward;
		int haircolor;
		int effect;
		int key;
	};

	union
	{
		unsigned char gender;
		unsigned char scrollx;
	};

	union
	{
		unsigned char scrolly;
		unsigned char dual_wield_dollgraphic;
	};

	short levelreq;
	short classreq;

	short strreq;
	short intreq;
	short wisreq;
	short agireq;
	short conreq;
	short chareq;

	unsigned char weight;

	typename EIF::Size size;

	EIF_Data_Base() : id(0), graphic(0), type(EIF::Static), subtype(EIF::None), special(EIF::Lore),
	hp(0), tp(0), mindam(0), maxdam(0), accuracy(0), evade(0), armor(0), str(0), intl(0), wis(0),
	agi(0), con(0), cha(0), light(0), dark(0), earth(0), air(0), water(0), fire(0), scrollmap(0),
	gender(0), scrolly(0), levelreq(0), classreq(0), strreq(0), intreq(0), wisreq(0), agireq(0),
	conreq(0), chareq(0), weight(0), size(EIF::Size1x1) { }

	explicit operator bool() const
	{
		return id != 0;
	}
};

/**
 * Loads and stores information on all items from an EIF file
 */
class EIF
{
	public:
		enum Type
		{
			Static,
			UnknownType1,
			Money,
			Heal,
			Teleport,
			Spell,
			EXPReward,
			StatReward,
			SkillReward,
			Key,
			Weapon,
			Shield,
			Armor,
			Hat,
			Boots,
			Gloves,
			Accessory,
			Belt,
			Necklace,
			Ring,
			Armlet,
			Bracer,
			Beer,
			EffectPotion,
			HairDye,
			CureCurse
		};

		enum SubType
		{
			None,
			Ranged,
			Arrows,
			Wings,
			TwoHanded
		};

		enum Special
		{
			Normal,
			Rare, // ?
			UnknownSpecial2,
			Unique, // ?
			Lore,
			Cursed
		};

		enum Size
		{
			Size1x1,
			Size1x2,
			Size1x3,
			Size1x4,
			Size2x1,
			Size2x2,
			Size2x3,
			Size2x4,
		};

		static int SizeTiles(Size size)
		{
			switch (size)
			{
				case Size1x1: return 1;
				case Size1x2: return 2;
				case Size1x3: return 3;
				case Size1x4: return 4;
				case Size2x1: return 2;
				case Size2x2: return 4;
				case Size2x3: return 6;
				case Size2x4: return 8;
				default:      return 0;
			}
		}

		using data_t = EIF_Data;
		static const int DATA_SIZE = 58;
		static const int FILE_MAX_ENTRIES = 900;
		std::array<unsigned char, 4> rid;
		std::array<unsigned char, 2> len;
		std::vector<EIF_Data> data;
		std::vector<Pub_File> files;

		EIF(const std::string& filename, bool auto_split) { Read(filename.c_str(), auto_split); }

		void Read(const std::string& filename, bool auto_split);

		EIF_Data& Get(unsigned int id);
		const EIF_Data& Get(unsigned int id) const;

		unsigned int GetKey(int keynum) const;
};

/**
 * One NPC record in an ENF object
 */
template <class ENF> struct ENF_Data_Base
{
	int id;
	std::string name;
	int graphic;

	short boss;
	short child;
	typename ENF::Type type;

	short vendor_id;

	int hp;
	unsigned short exp;
	short mindam;
	short maxdam;

	short accuracy;
	short evade;
	short armor;

	ENF_Data_Base() : id(0), graphic(0), boss(0), child(0), type(ENF::NPC), vendor_id(0),
	hp(0), exp(0), mindam(0), maxdam(0), accuracy(0), evade(0), armor(0) { }

	explicit operator bool() const
	{
		return id != 0;
	}
};

/**
 * Loads and stores information on all NPCs from an ENF file
 */
class ENF
{
	public:
		enum Type
		{
			NPC,
			Passive,
			Aggressive,
			Unknown1,
			Unknown2,
			Unknown3,
			Shop,
			Inn,
			Unknown4,
			Bank,
			Barber,
			Guild,
			Priest,
			Law,
			Skills,
			Quest
		};

		using data_t = ENF_Data;
		static const int DATA_SIZE = 39;
		static const int FILE_MAX_ENTRIES = 900;
		std::array<unsigned char, 4> rid;
		std::array<unsigned char, 2> len;
		std::vector<ENF_Data> data;
		std::vector<Pub_File> files;

		ENF(const std::string& filename, bool auto_split) { Read(filename.c_str(), auto_split); }

		void Read(const std::string& filename, bool auto_split);

		ENF_Data& Get(unsigned int id);
		const ENF_Data& Get(unsigned int id) const;
};

/**
 * One spell record in an ESF object
 */
template <class ESF> struct ESF_Data_Base
{
	int id;
	std::string name;
	std::string shout;

	short icon;
	short graphic;

	short tp;
	short sp;

	unsigned char cast_time;

	typename ESF::Type type;
	typename ESF::TargetRestrict target_restrict;
	typename ESF::Target target;

	short mindam;
	short maxdam;
	short accuracy;
	short hp;

	ESF_Data_Base() : id(0), icon(0), graphic(0), tp(0), sp(0), cast_time(0), type(ESF::Damage),
	target_restrict(ESF::NPCOnly), target(ESF::Normal), mindam(0), maxdam(0), accuracy(0), hp(0) { }

	explicit operator bool() const
	{
		return id != 0;
	}
};

/**
 * Loads and stores information on all spells from an ESF file
 */
class ESF
{
	public:
		enum Type
		{
			Heal,
			Damage,
			Bard
		};

		enum TargetRestrict
		{
			NPCOnly,
			Friendly,
			Opponent
		};

		enum Target
		{
			Normal,
			Self,
			Unknown1,
			Group
		};

		using data_t = ESF_Data;
		static const int DATA_SIZE = 51;
		static const int FILE_MAX_ENTRIES = 900;
		std::array<unsigned char, 4> rid;
		std::array<unsigned char, 2> len;
		std::vector<ESF_Data> data;
		std::vector<Pub_File> files;

		ESF(const std::string& filename, bool auto_split) { Read(filename.c_str(), auto_split); }

		void Read(const std::string& filename, bool auto_split);

		ESF_Data& Get(unsigned int id);
		const ESF_Data& Get(unsigned int id) const;
};

/**
 * One class record in an ECF object
 */
template <class ECF> struct ECF_Data_Base
{
	int id;
	std::string name;

	unsigned char base;
	unsigned char type;

	short str;
	short intl;
	short wis;
	short agi;
	short con;
	short cha;

	ECF_Data_Base() : id(0), base(4), type(4), str(0), intl(0), wis(0), agi(0), con(0), cha(0) { }

	explicit operator bool() const
	{
		return id != 0;
	}
};

/**
 * Loads and stores information on all classes from an ECF file
 */
class ECF
{
	public:
		using data_t = ECF_Data;
		static const int DATA_SIZE = 14;
		static const int FILE_MAX_ENTRIES = 250;
		static const int FILE_MAX_ENTRIES_V2 = 900;
		std::array<unsigned char, 4> rid;
		std::array<unsigned char, 2> len;
		std::vector<ECF_Data> data;
		std::vector<Pub_File> files;

		ECF(const std::string& filename, bool auto_split) { Read(filename.c_str(), auto_split); }

		void Read(const std::string& filename, bool auto_split);

		ECF_Data& Get(unsigned int id);
		const ECF_Data& Get(unsigned int id) const;
};

#endif // EODATA_HPP_INCLUDED
