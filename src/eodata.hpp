
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
		PtrVector<EIF_Data> data;
		EIF(std::string filename) { Read(filename); }
		void Read(std::string filename);

		EIF_Data *Get(unsigned int id);
		unsigned int GetKey(int keynum);

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
			SCRIPT_REGISTER_ENUM_VALUE(CureCurse);
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

		SCRIPT_REGISTER_ENUM("EIF_Size")
			SCRIPT_REGISTER_ENUM_VALUE(Size1x1);
			SCRIPT_REGISTER_ENUM_VALUE(Size1x2);
			SCRIPT_REGISTER_ENUM_VALUE(Size1x3);
			SCRIPT_REGISTER_ENUM_VALUE(Size1x4);
			SCRIPT_REGISTER_ENUM_VALUE(Size2x1);
			SCRIPT_REGISTER_ENUM_VALUE(Size2x2);
			SCRIPT_REGISTER_ENUM_VALUE(Size2x3);
			SCRIPT_REGISTER_ENUM_VALUE(Size2x4);
		SCRIPT_REGISTER_ENUM_END()

		SCRIPT_REGISTER_FACTORY("EIF @f(string filename)", ScriptFactory);

		SCRIPT_REGISTER_VARIABLE("PtrVector<EIF_Data>", data);
		SCRIPT_REGISTER_FUNCTION("void Read(string filename)", Read);

		SCRIPT_REGISTER_FUNCTION("EIF_Data @Get(uint)", Get);
		SCRIPT_REGISTER_FUNCTION("uint GetKey(int)", GetKey);
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
		SCRIPT_REGISTER_VARIABLE("uint8", str);
		SCRIPT_REGISTER_VARIABLE("uint8", intl);
		SCRIPT_REGISTER_VARIABLE("uint8", wis);
		SCRIPT_REGISTER_VARIABLE("uint8", agi);
		SCRIPT_REGISTER_VARIABLE("uint8", con);
		SCRIPT_REGISTER_VARIABLE("uint8", cha);
		SCRIPT_REGISTER_VARIABLE("uint8", light);
		SCRIPT_REGISTER_VARIABLE("uint8", dark);
		SCRIPT_REGISTER_VARIABLE("uint8", earth);
		SCRIPT_REGISTER_VARIABLE("uint8", air);
		SCRIPT_REGISTER_VARIABLE("uint8", water);
		SCRIPT_REGISTER_VARIABLE("uint8", fire);
		SCRIPT_REGISTER_VARIABLE("int", scrollmap);
		SCRIPT_REGISTER_VARIABLE("int", dollgraphic);
		SCRIPT_REGISTER_VARIABLE("int", expreward);
		SCRIPT_REGISTER_VARIABLE("int", haircolor);
		SCRIPT_REGISTER_VARIABLE("int", effect);
		SCRIPT_REGISTER_VARIABLE("uint8", gender);
		SCRIPT_REGISTER_VARIABLE("uint8", scrollx);
		SCRIPT_REGISTER_VARIABLE("uint8", scrolly);
		SCRIPT_REGISTER_VARIABLE("int16", levelreq);
		SCRIPT_REGISTER_VARIABLE("int16", classreq);
		SCRIPT_REGISTER_VARIABLE("int16", strreq);
		SCRIPT_REGISTER_VARIABLE("int16", intreq);
		SCRIPT_REGISTER_VARIABLE("int16", wisreq);
		SCRIPT_REGISTER_VARIABLE("int16", agireq);
		SCRIPT_REGISTER_VARIABLE("int16", conreq);
		SCRIPT_REGISTER_VARIABLE("int16", chareq);
		SCRIPT_REGISTER_VARIABLE("uint8", weight);
		SCRIPT_REGISTER_VARIABLE("EIF_Size", size);
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
	unsigned short exp;
	short mindam;
	short maxdam;

	short accuracy;
	short evade;
	short armor;

	ENF_Data() : id(0), graphic(0), boss(0), child(0), type(ENF::NPC), hp(0), exp(0), mindam(0), maxdam(0), accuracy(0), evade(0), armor(0) { }

	SCRIPT_REGISTER_REF_DF(ENF_Data)
		SCRIPT_REGISTER_VARIABLE("int", id);
		SCRIPT_REGISTER_VARIABLE("string", name);
		SCRIPT_REGISTER_VARIABLE("int", graphic);
		SCRIPT_REGISTER_VARIABLE("int16", boss);
		SCRIPT_REGISTER_VARIABLE("int16", child);
		SCRIPT_REGISTER_VARIABLE("ENF_Type", type);
		SCRIPT_REGISTER_VARIABLE("int", hp);
		SCRIPT_REGISTER_VARIABLE("uint16", exp);
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
		enum Type
		{
			Damage,
			Heal,
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
		PtrVector<ESF_Data> data;
		ESF(std::string filename) { Read(filename); }
		void Read(std::string filename);

	static ESF *ScriptFactory(std::string filename) { return new ESF(filename); }

	SCRIPT_REGISTER_REF(ESF)
		SCRIPT_REGISTER_ENUM("ESF_Type")
			SCRIPT_REGISTER_ENUM_VALUE(Damage);
			SCRIPT_REGISTER_ENUM_VALUE(Heal);
			SCRIPT_REGISTER_ENUM_VALUE(Bard);
		SCRIPT_REGISTER_ENUM_END()

		SCRIPT_REGISTER_ENUM("ESF_TargetRestrict")
			SCRIPT_REGISTER_ENUM_VALUE(Any);
			SCRIPT_REGISTER_ENUM_VALUE(Friendly);
			SCRIPT_REGISTER_ENUM_VALUE(Opponent);
		SCRIPT_REGISTER_ENUM_END()

		SCRIPT_REGISTER_ENUM("ESF_Target")
			SCRIPT_REGISTER_ENUM_VALUE(Normal);
			SCRIPT_REGISTER_ENUM_VALUE(Self);
			SCRIPT_REGISTER_ENUM_VALUE(Unknown1);
			SCRIPT_REGISTER_ENUM_VALUE(Group);
		SCRIPT_REGISTER_ENUM_END()

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

	SCRIPT_REGISTER_REF_DF(ESF_Data)
		SCRIPT_REGISTER_VARIABLE("int", id);
		SCRIPT_REGISTER_VARIABLE("string", name);
		SCRIPT_REGISTER_VARIABLE("string", shout);
		SCRIPT_REGISTER_VARIABLE("int16", icon);
		SCRIPT_REGISTER_VARIABLE("int16", graphic);
		SCRIPT_REGISTER_VARIABLE("int16", tp);
		SCRIPT_REGISTER_VARIABLE("int16", sp);
		SCRIPT_REGISTER_VARIABLE("uint8", cast_time);
		SCRIPT_REGISTER_VARIABLE("ESF_Type", type);
		SCRIPT_REGISTER_VARIABLE("ESF_TargetRestrict", target_restrict);
		SCRIPT_REGISTER_VARIABLE("ESF_Target", target);
		SCRIPT_REGISTER_VARIABLE("int16", mindam);
		SCRIPT_REGISTER_VARIABLE("int16", maxdam);
		SCRIPT_REGISTER_VARIABLE("int16", accuracy);
		SCRIPT_REGISTER_VARIABLE("int16", hp);
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

	unsigned char base;
	unsigned char type;

	short str;
	short intl;
	short wis;
	short agi;
	short con;
	short cha;

	ECF_Data() : id(0), base(0), type(0), str(0), intl(0), wis(0), agi(0), con(0), cha(9) { }

	SCRIPT_REGISTER_REF_DF(ECF_Data)
		SCRIPT_REGISTER_VARIABLE("int", id);
		SCRIPT_REGISTER_VARIABLE("string", name);
	SCRIPT_REGISTER_END()
};

#endif // EODATA_HPP_INCLUDED
