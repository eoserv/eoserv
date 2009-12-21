
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
class EIF : public Shared
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
			UnknownType6
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
		PtrVector<EIF_Data> data;
		EIF(std::string filename) { Read(filename); }
		void Read(std::string filename);

		EIF_Data *Get(unsigned int id);

	static EIF *ScriptFactory(std::string filename) { return new EIF(filename); }

	SCRIPT_REGISTER_REF(EIF)
		SCRIPT_REGISTER_ENUM("EIF_Type")
			SCRIPT_REGISTER_ENUM_VALUE(Static);
			SCRIPT_REGISTER_ENUM_VALUE(UnknownType1);
			SCRIPT_REGISTER_ENUM_VALUE(Money);
			SCRIPT_REGISTER_ENUM_VALUE(Heal);
			SCRIPT_REGISTER_ENUM_VALUE(Teleport);
			SCRIPT_REGISTER_ENUM_VALUE(Spell);
			SCRIPT_REGISTER_ENUM_VALUE(EXPReward);
			SCRIPT_REGISTER_ENUM_VALUE(StatReward);
			SCRIPT_REGISTER_ENUM_VALUE(SkillReward);
			SCRIPT_REGISTER_ENUM_VALUE(Key);
			SCRIPT_REGISTER_ENUM_VALUE(Weapon);
			SCRIPT_REGISTER_ENUM_VALUE(Shield);
			SCRIPT_REGISTER_ENUM_VALUE(Armor);
			SCRIPT_REGISTER_ENUM_VALUE(Hat);
			SCRIPT_REGISTER_ENUM_VALUE(Boots);
			SCRIPT_REGISTER_ENUM_VALUE(Gloves);
			SCRIPT_REGISTER_ENUM_VALUE(Accessory);
			SCRIPT_REGISTER_ENUM_VALUE(Belt);
			SCRIPT_REGISTER_ENUM_VALUE(Necklace);
			SCRIPT_REGISTER_ENUM_VALUE(Ring);
			SCRIPT_REGISTER_ENUM_VALUE(Armlet);
			SCRIPT_REGISTER_ENUM_VALUE(Bracer);
			SCRIPT_REGISTER_ENUM_VALUE(Beer);
			SCRIPT_REGISTER_ENUM_VALUE(EffectPotion);
			SCRIPT_REGISTER_ENUM_VALUE(HairDye);
			SCRIPT_REGISTER_ENUM_VALUE(OtherPotion);
			SCRIPT_REGISTER_ENUM_VALUE(CureCurse);
			SCRIPT_REGISTER_ENUM_VALUE(UnknownType3);
			SCRIPT_REGISTER_ENUM_VALUE(UnknownType4);
			SCRIPT_REGISTER_ENUM_VALUE(UnknownType5);
			SCRIPT_REGISTER_ENUM_VALUE(UnknownType6);
		SCRIPT_REGISTER_ENUM_END()

		SCRIPT_REGISTER_ENUM("EIF_SubType")
			SCRIPT_REGISTER_ENUM_VALUE(None);
			SCRIPT_REGISTER_ENUM_VALUE(Ranged);
			SCRIPT_REGISTER_ENUM_VALUE(Arrows);
			SCRIPT_REGISTER_ENUM_VALUE(Wings);
		SCRIPT_REGISTER_ENUM_END()

		SCRIPT_REGISTER_ENUM("EIF_Special")
			SCRIPT_REGISTER_ENUM_VALUE(Normal);
			SCRIPT_REGISTER_ENUM_VALUE(Rare);
			SCRIPT_REGISTER_ENUM_VALUE(UnknownSpecial2);
			SCRIPT_REGISTER_ENUM_VALUE(Unique);
			SCRIPT_REGISTER_ENUM_VALUE(Lore);
			SCRIPT_REGISTER_ENUM_VALUE(Cursed);
		SCRIPT_REGISTER_ENUM_END()

		SCRIPT_REGISTER_FACTORY("EIF @f(string filename)", ScriptFactory);

		SCRIPT_REGISTER_VARIABLE("PtrVector<EIF_Data>", data);
		SCRIPT_REGISTER_FUNCTION("void Read(string filename)", Read);

		SCRIPT_REGISTER_FUNCTION("EIF_Data @Get(uint)", Get);
	SCRIPT_REGISTER_END()
};

/**
 * One item record in an EIF object
 */
struct EIF_Data : public Shared
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

	SCRIPT_REGISTER_REF_DF(EIF_Data)
		SCRIPT_REGISTER_VARIABLE("int", id);
		SCRIPT_REGISTER_VARIABLE("string", name);
		SCRIPT_REGISTER_VARIABLE("int", graphic);
		SCRIPT_REGISTER_VARIABLE("EIF_Type", type);
		SCRIPT_REGISTER_VARIABLE("EIF_SubType", subtype);
		SCRIPT_REGISTER_VARIABLE("EIF_Special", special);
		SCRIPT_REGISTER_VARIABLE("int16", hp);
		SCRIPT_REGISTER_VARIABLE("int16", tp);
		SCRIPT_REGISTER_VARIABLE("int16", mindam);
		SCRIPT_REGISTER_VARIABLE("int16", maxdam);
		SCRIPT_REGISTER_VARIABLE("int16", accuracy);
		SCRIPT_REGISTER_VARIABLE("int16", evade);
		SCRIPT_REGISTER_VARIABLE("int16", armor);
		SCRIPT_REGISTER_VARIABLE("int16", str);
		SCRIPT_REGISTER_VARIABLE("int16", intl);
		SCRIPT_REGISTER_VARIABLE("int16", wis);
		SCRIPT_REGISTER_VARIABLE("int16", agi);
		SCRIPT_REGISTER_VARIABLE("int16", con);
		SCRIPT_REGISTER_VARIABLE("int16", cha);
		SCRIPT_REGISTER_VARIABLE("int16", scrollmap);
		SCRIPT_REGISTER_VARIABLE("int16", dollgraphic);
		SCRIPT_REGISTER_VARIABLE("int16", expreward);
		SCRIPT_REGISTER_VARIABLE("int16", haircolor);
		SCRIPT_REGISTER_VARIABLE("int16", gender);
		SCRIPT_REGISTER_VARIABLE("int16", scrollx);
		SCRIPT_REGISTER_VARIABLE("int16", scrolly);
		SCRIPT_REGISTER_VARIABLE("int16", classreq);
		SCRIPT_REGISTER_VARIABLE("int16", weight);
	SCRIPT_REGISTER_END()
};

/**
 * Loads and stores information on all NPCs from an ENF file
 */
class ENF : public Shared
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
		PtrVector<ENF_Data> data;
		ENF(std::string filename) { Read(filename); }
		void Read(std::string filename);

		ENF_Data *Get(unsigned int id);

	static ENF *ScriptFactory(std::string filename) { return new ENF(filename); }

	SCRIPT_REGISTER_REF(ENF)
		SCRIPT_REGISTER_ENUM("ENF_Type")
			SCRIPT_REGISTER_ENUM_VALUE(NPC);
			SCRIPT_REGISTER_ENUM_VALUE(Passive);
			SCRIPT_REGISTER_ENUM_VALUE(Aggressive);
			SCRIPT_REGISTER_ENUM_VALUE(Unknown1);
			SCRIPT_REGISTER_ENUM_VALUE(Unknown2);
			SCRIPT_REGISTER_ENUM_VALUE(Unknown3);
			SCRIPT_REGISTER_ENUM_VALUE(Shop);
			SCRIPT_REGISTER_ENUM_VALUE(Inn);
			SCRIPT_REGISTER_ENUM_VALUE(Unknown4);
			SCRIPT_REGISTER_ENUM_VALUE(Bank);
			SCRIPT_REGISTER_ENUM_VALUE(Barber);
			SCRIPT_REGISTER_ENUM_VALUE(Guild);
			SCRIPT_REGISTER_ENUM_VALUE(Priest);
			SCRIPT_REGISTER_ENUM_VALUE(Law);
			SCRIPT_REGISTER_ENUM_VALUE(Skills);
			SCRIPT_REGISTER_ENUM_VALUE(Quest);
		SCRIPT_REGISTER_ENUM_END()

		SCRIPT_REGISTER_FACTORY("ENF @f(string filename)", ScriptFactory);

		SCRIPT_REGISTER_VARIABLE("PtrVector<ENF_Data>", data);
		SCRIPT_REGISTER_FUNCTION("void Read(string filename)", Read);

		SCRIPT_REGISTER_FUNCTION("ENF_Data @Get(uint)", Get);
	SCRIPT_REGISTER_END()
};

/**
 * One NPC record in an ENF object
 */
struct ENF_Data : public Shared
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

	SCRIPT_REGISTER_REF_DF(ENF_Data)
		SCRIPT_REGISTER_VARIABLE("int", id);
		SCRIPT_REGISTER_VARIABLE("string", name);
		SCRIPT_REGISTER_VARIABLE("int", graphic);
		SCRIPT_REGISTER_VARIABLE("int16", boss);
		SCRIPT_REGISTER_VARIABLE("int16", child);
		SCRIPT_REGISTER_VARIABLE("ENF_Type", type);
		SCRIPT_REGISTER_VARIABLE("int", hp);
		SCRIPT_REGISTER_VARIABLE("int", exp);
		SCRIPT_REGISTER_VARIABLE("int16", mindam);
		SCRIPT_REGISTER_VARIABLE("int16", maxdam);
		SCRIPT_REGISTER_VARIABLE("int16", accuracy);
		SCRIPT_REGISTER_VARIABLE("int16", evade);
		SCRIPT_REGISTER_VARIABLE("int16", armor);
	SCRIPT_REGISTER_END()
};

/**
 * Loads and stores information on all spells from an ESF file
 */
class ESF : public Shared
{
	public:
		static const int DATA_SIZE = 51;
		unsigned char rid[4];
		unsigned char len[2];
		PtrVector<ESF_Data> data;
		ESF(std::string filename) { Read(filename); }
		void Read(std::string filename);

	static ESF *ScriptFactory(std::string filename) { return new ESF(filename); }

	SCRIPT_REGISTER_REF(ESF)
		SCRIPT_REGISTER_FACTORY("ESF @f(string filename)", ScriptFactory);

		SCRIPT_REGISTER_VARIABLE("PtrVector<ESF_Data>", data);
		SCRIPT_REGISTER_FUNCTION("void Read(string filename)", Read);
	SCRIPT_REGISTER_END()
};

/**
 * One spell record in an ESF object
 */
struct ESF_Data : public Shared
{
	int id;
	std::string name;
	std::string shout;

	ESF_Data() : id(0) {}

	SCRIPT_REGISTER_REF_DF(ESF_Data)
		SCRIPT_REGISTER_VARIABLE("int", id);
		SCRIPT_REGISTER_VARIABLE("string", name);
		SCRIPT_REGISTER_VARIABLE("string", shout);
	SCRIPT_REGISTER_END()
};

/**
 * Loads and stores information on all classes from an ECF file
 */
class ECF : public Shared
{
	public:
		static const int DATA_SIZE = 14;
		unsigned char rid[4];
		unsigned char len[2];
		PtrVector<ECF_Data> data;
		ECF(std::string filename) { Read(filename); }
		void Read(std::string filename);

	static ECF *ScriptFactory(std::string filename) { return new ECF(filename); }

	SCRIPT_REGISTER_REF(ECF)
		SCRIPT_REGISTER_FACTORY("ECF @f(string filename)", ScriptFactory);

		SCRIPT_REGISTER_VARIABLE("PtrVector<ECF_Data>", data);
		SCRIPT_REGISTER_FUNCTION("void Read(string filename)", Read);
	SCRIPT_REGISTER_END()
};

/**
 * One class record in an ECF object
 */
struct ECF_Data : public Shared
{
	int id;
	std::string name;

	ECF_Data() : id(0) {}

	SCRIPT_REGISTER_REF_DF(ECF_Data)
		SCRIPT_REGISTER_VARIABLE("int", id);
		SCRIPT_REGISTER_VARIABLE("string", name);
	SCRIPT_REGISTER_END()
};

#endif // EODATA_HPP_INCLUDED
