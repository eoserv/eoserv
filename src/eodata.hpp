
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef EODATA_HPP_INCLUDED
#define EODATA_HPP_INCLUDED

#include "stdafx.h"

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
			OtherPotion,
			CureCurse,
			UnknownType3,
			UnknownType4,
			UnknownType5,
			UnknownType6,
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

		static const int DATA_SIZE = 58;
		unsigned char rid[4];
		unsigned char len[2];
		std::vector<EIF_Data> data;
		EIF(std::string filename);

		EIF_Data *Get(unsigned int id);
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

	short str;
	short intl;
	short wis;
	short agi;
	short con;
	short cha;

	union
	{
		short scrollmap;
		short dollgraphic;
		short expreward;
		short haircolor;
	};

	union
	{
		short gender;
		short scrollx;
	};

	union
	{
		short scrolly;
	};

	short classreq;

	short weight;

	EIF_Data() : id(0), graphic(0), type(EIF::Static), subtype(EIF::None), special(EIF::Normal),
	hp(0), tp(0), mindam(0), maxdam(0), accuracy(0), evade(0), armor(0), str(0), intl(0), wis(0),
	agi(0), con(0), cha(0), scrollmap(0), gender(0), scrolly(0), classreq(0), weight(0) {};
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
		std::vector<ENF_Data> data;
		ENF(std::string filename);

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
	int exp;
	short mindam;
	short maxdam;

	short accuracy;
	short evade;
	short armor;

	ENF_Data() : id(0), graphic(0), boss(0), child(0), type(ENF::NPC), hp(0), exp(0), mindam(0), maxdam(0), accuracy(0), evade(0), armor(0) {}
};

/**
 * Loads and stores information on all spells from an ESF file
 */
class ESF
{
	public:
		static const int DATA_SIZE = 51;
		unsigned char rid[4];
		unsigned char len[2];
		std::vector<ESF_Data> data;
		ESF(std::string filename);

};

/**
 * One spell record in an ESF object
 */
struct ESF_Data
{
	int id;
	std::string name;
	std::string shout;

	ESF_Data() : id(0) {}
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
		std::vector<ECF_Data> data;
		ECF(std::string filename);

};

/**
 * One class record in an ECF object
 */
struct ECF_Data
{
	int id;
	std::string name;

	ECF_Data() : id(0) {}
};

#endif // EODATA_HPP_INCLUDED
