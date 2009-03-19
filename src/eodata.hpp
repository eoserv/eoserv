#ifndef EODATA_HPP_INCLUDED
#define EODATA_HPP_INCLUDED

#include <string>
#include <vector>
#include <cstdio>

// TODO: this

class EIF_Data;
class ENF_Data;
class ESF_Data;
class ECF_Data;

class EIF;
class ENF;
class ESF;
class ECF;

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
		EIF_Data *nulldata;
		EIF(std::string filename);

		int GetType(unsigned int id);
		int GetGraphic(unsigned int id);
		int GetDollGraphic(unsigned int id);
};


struct EIF_Data
{
	int id;
	std::string name;
	int graphic;
	EIF::Type type;
	int classreq;
	EIF::Special special;
	int hp;
	int tp;
	int mindam;
	int maxdam;
	int accuracy;
	int evade;
	int armor;

	int str;
	int intl;
	int wis;
	int agi;
	int con;
	int cha;

	union
	{
		int scrollmap;
		int dollgraphic;
	};
	int scrollx;
	int scrolly;
};

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
		ENF_Data *nulldata;
		ENF(std::string filename);

};

struct ENF_Data
{
	int id;
	std::string name;
	int graphic;

	int boss;
	int child;
	ENF::Type type;

	int exp;
};

class ESF
{
	public:
		static const int DATA_SIZE = 51;
		unsigned char rid[4];
		unsigned char len[2];
		std::vector<ESF_Data> data;
		ESF_Data *nulldata;
		ESF(std::string filename);

};

struct ESF_Data
{
	int id;
	std::string name;
};

class ECF
{
	public:
		static const int DATA_SIZE = 14;
		unsigned char rid[4];
		unsigned char len[2];
		std::vector<ECF_Data> data;
		ECF_Data *nulldata;
		ECF(std::string filename);

};

struct ECF_Data
{
	int id;
	std::string name;
};

#endif // EODATA_HPP_INCLUDED
