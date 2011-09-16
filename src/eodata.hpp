
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef EODATA_HPP_INCLUDED
#define EODATA_HPP_INCLUDED

#include "fwd/eodata.hpp"

#include <string>
#include <vector>

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
			Wings
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

		static const int DATA_SIZE = 58;
		unsigned char rid[4];
		unsigned char len[2];
		std::vector<EIF_Data *> data;
		EIF(std::string filename) { Read(filename); }
		void Read(std::string filename);

		EIF_Data *Get(unsigned int id);
		unsigned int GetKey(int keynum);
};

/**
 * One item record in an EIF object
 */
struct EIF_Data
{
	int id;
	std::string name;
	short graphic;
	EIF::Type type;
	EIF::SubType subtype;

	EIF::Special special;
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

	EIF::Size size;

	EIF_Data() : id(0), graphic(0), type(EIF::Static), subtype(EIF::None), special(EIF::Normal),
	hp(0), tp(0), mindam(0), maxdam(0), accuracy(0), evade(0), armor(0), str(0), intl(0), wis(0),
	agi(0), con(0), cha(0), light(0), dark(0), earth(0), air(0), water(0), fire(0), scrollmap(0),
	gender(0), scrolly(0), levelreq(0), classreq(0), strreq(0), intreq(0), wisreq(0), agireq(0),
	conreq(0), chareq(0), weight(0), size(EIF::Size1x1) { }
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

		static const int DATA_SIZE = 39;
		unsigned char rid[4];
		unsigned char len[2];
		std::vector<ENF_Data *> data;
		ENF(std::string filename) { Read(filename); }
		void Read(std::string filename);

		ENF_Data *Get(unsigned int id);
};

/**
 * One NPC record in an ENF object
 */
struct ENF_Data
{
	int id;
	std::string name;
	int graphic;

	short boss;
	short child;
	ENF::Type type;

	int hp;
	unsigned short exp;
	short mindam;
	short maxdam;

	short accuracy;
	short evade;
	short armor;

	ENF_Data() : id(0), graphic(0), boss(0), child(0), type(ENF::NPC), hp(0), exp(0), mindam(0), maxdam(0), accuracy(0), evade(0), armor(0) { }
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
			Any,
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

		static const int DATA_SIZE = 51;
		unsigned char rid[4];
		unsigned char len[2];
		std::vector<ESF_Data *> data;
		ESF(std::string filename) { Read(filename); }
		void Read(std::string filename);

		ESF_Data *Get(unsigned int id);
};

/**
 * One spell record in an ESF object
 */
struct ESF_Data
{
	int id;
	std::string name;
	std::string shout;

	short icon;
	short graphic;

	short tp;
	short sp;

	unsigned char cast_time;

	ESF::Type type;
	ESF::TargetRestrict target_restrict;
	ESF::Target target;

	short mindam;
	short maxdam;
	short accuracy;
	short hp;

	ESF_Data() : id(0), icon(0), graphic(0), tp(0), sp(0), cast_time(0), type(ESF::Damage),
	target_restrict(ESF::Any), target(ESF::Normal), mindam(0), maxdam(0), accuracy(0), hp(0) { }
};

/**
 * Loads and stores information on all classes from an ECF file
 */
class ECF
{
	public:
		static const int DATA_SIZE = 14;
		unsigned char rid[4];
		unsigned char len[2];
		std::vector<ECF_Data *> data;
		ECF(std::string filename) { Read(filename); }
		void Read(std::string filename);
};

/**
 * One class record in an ECF object
 */
struct ECF_Data
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

	ECF_Data() : id(0), base(0), type(0), str(0), intl(0), wis(0), agi(0), con(0), cha(0) { }
};

#endif // EODATA_HPP_INCLUDED
