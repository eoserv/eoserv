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
			Potion,
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
			UnknownType2,
			UnknownType3,
			UnknownType4,
			UnknownType5,
			UnknownType6,
		};

		enum Special
		{
			Normal,
			UnknownSpecial1,
			UnknownSpecial2,
			UnknownSpecial3,
			UnknownSpecial4,
			Lore,
			Cursed
		};

		static const int DATA_SIZE = 58;
		char rid[6];
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

	int scrollx;
	union
	{
		int dollgraphic;
		int scrolly;
	};
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
		char rid[6];
		std::vector<ENF_Data> data;
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
		char rid[6];
		std::vector<ESF_Data> data;
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
		char rid[6];
		std::vector<ECF_Data> data;
		ECF(std::string filename);

};

struct ECF_Data
{
	int id;
	std::string name;
};

#endif // EODATA_HPP_INCLUDED
