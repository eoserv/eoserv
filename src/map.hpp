
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef MAP_HPP_INCLUDED
#define MAP_HPP_INCLUDED

#include "stdafx.h"

/**
 * Object representing an item on the floor of a map
 */
struct Map_Item : public Shared
{
	short uid;
	short id;
	int amount;
	unsigned char x;
	unsigned char y;
	unsigned int owner; // Player ID
	double unprotecttime;

	Map_Item(short uid_, short id_, int amount_, unsigned char x_, unsigned char y_, unsigned int owner_, double unprotecttime_)
	 : uid(uid_), id(id_), amount(amount_), x(x_), y(y_), owner(owner_), unprotecttime(unprotecttime_) { }

	SCRIPT_REGISTER_REF(Map_Item)

	SCRIPT_REGISTER_END()
};

/**
 * Object representing a warp tile on a map, as well as storing door state
 */
struct Map_Warp : public Shared
{
	short map;
	unsigned char x;
	unsigned char y;
	unsigned char levelreq;

	enum WarpSpec
	{
		NoDoor,
		Door,
		LockedSilver,
		LockedCrystal,
		LockedWraith
	};

	WarpSpec spec;
	bool open;

	Map_Warp() : spec(Map_Warp::NoDoor), open(false) {}

	SCRIPT_REGISTER_REF_DF(Map_Warp)
		SCRIPT_REGISTER_ENUM("WarpSpec");
			SCRIPT_REGISTER_ENUM_VALUE("WarpSpec", NoDoor);
			SCRIPT_REGISTER_ENUM_VALUE("WarpSpec", Door);
			SCRIPT_REGISTER_ENUM_VALUE("WarpSpec", LockedSilver);
			SCRIPT_REGISTER_ENUM_VALUE("WarpSpec", LockedCrystal);
			SCRIPT_REGISTER_ENUM_VALUE("WarpSpec", LockedWraith);
	SCRIPT_REGISTER_END()
};

/**
 * Object representing one tile on a map
 */
struct Map_Tile : public Shared
{
	enum TileSpec
	{
		None = -1,
		Wall,
		ChairDown,
		ChairLeft,
		ChairRight,
		ChairUp,
		ChairDownRight,
		ChairUpLeft,
		ChairAll,
		Door,
		Chest,
		Unknown1,
		Unknown2,
		Unknown3,
		Unknown4,
		Unknown5,
		Unknown6,
		BankVault,
		NPCBoundary,
		MapEdge,
		FakeWall,
		Board1,
		Board2,
		Board3,
		Board4,
		Board5,
		Board6,
		Board7,
		Board8,
		Jukebox,
		Jump,
		Water,
		Unknown7,
		Arena,
		AmbientSource,
		Spikes1,
		Spikes2,
		Spikes3
	};

	TileSpec tilespec;

	Map_Warp *warp;

	Map_Tile() : tilespec(Map_Tile::None), warp(0) { }

	bool Walkable(bool npc = false)
	{
		if (this->warp && npc)
		{
			return false;
		}

		switch (this->tilespec)
		{
			case Wall:
			case ChairDown:
			case ChairLeft:
			case ChairRight:
			case ChairUp:
			case ChairDownRight:
			case ChairUpLeft:
			case ChairAll:
			case Chest:
			case BankVault:
			case MapEdge:
			case Board1:
			case Board2:
			case Board3:
			case Board4:
			case Board5:
			case Board6:
			case Board7:
			case Jukebox:
				return false;
			case NPCBoundary:
				return !npc;
			default:
				return true;
		}
	}

	SCRIPT_REGISTER_REF_DF(Map_Tile)
		SCRIPT_REGISTER_ENUM("TileSpec");
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", None);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Wall);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", ChairDown);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", ChairLeft);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", ChairRight);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", ChairUp);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", ChairDownRight);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", ChairUpLeft);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", ChairAll);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Door);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Chest);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Unknown1);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Unknown2);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Unknown3);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Unknown4);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Unknown5);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Unknown6);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", BankVault);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", NPCBoundary);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", MapEdge);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", FakeWall);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Board1);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Board2);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Board3);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Board4);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Board5);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Board6);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Board7);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Board8);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Jukebox);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Jump);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Water);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Unknown7);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Arena);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", AmbientSource);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Spikes1);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Spikes2);
			SCRIPT_REGISTER_ENUM_VALUE("TileSpec", Spikes3);
	SCRIPT_REGISTER_END()
};

/**
 * Object representing an item in a chest on a map
 */
struct Map_Chest_Item : public Shared
{
	short id;
	int amount;
	int slot;

	SCRIPT_REGISTER_REF_DF(Map_Chest_Item)

	SCRIPT_REGISTER_END()
};

/**
 * Object representing an item spawn in a chest on a map
 */
struct Map_Chest_Spawn : public Shared
{
	int spawnid;
	Map_Chest_Item *item;
	int slot;
	short time;
	double last_taken;

	SCRIPT_REGISTER_REF_DF(Map_Chest_Spawn)

	SCRIPT_REGISTER_END()
};

/**
 * Object representing a chest on a map
 */
struct Map_Chest : public Shared
{
	unsigned char x;
	unsigned char y;

	PtrList<Map_Chest_Item> items;
	PtrList<Map_Chest_Spawn> spawns;
	int slots;

	int maxchest;
	int chestslots;

	int AddItem(short item, int amount, int slot = 0);
	int DelItem(short item);

	void Update(Map *map, Character *exclude);

	SCRIPT_REGISTER_REF_DF(Map_Chest)

	SCRIPT_REGISTER_END()
};

/**
 * Contains all information about a map, holds reference to contained Characters and manages NPCs on it
 */
class Map : public Shared
{
	private:
		bool Load();
		void Unload();

	public:
		World *world;
		short id;
		char rid[4];
		bool pk;
		int filesize;
		unsigned char width;
		unsigned char height;
		bool scroll;
		unsigned char relog_x;
		unsigned char relog_y;
		std::string filename;
		PtrList<Character> characters;
		PtrVector<NPC> npcs;
		PtrVector<Map_Chest> chests;
		PtrList<Map_Item> items;
		PtrVector<PtrVector<Map_Tile> > tiles;
		bool exists;
		double jukebox_protect;
		std::string jukebox_player;

		Arena *arena;

		Map(int id, World *server);

		int GenerateItemID();
		unsigned char GenerateNPCIndex();

		void Enter(Character *, WarpAnimation animation = WARP_ANIMATION_NONE);
		void Leave(Character *, WarpAnimation animation = WARP_ANIMATION_NONE, bool silent = false);

		void Msg(Character *from, std::string message, bool echo = true);
		bool Walk(Character *from, Direction direction, bool admin = false);
		void Attack(Character *from, Direction direction);
		bool AttackPK(Character *from, Direction direction);
		void Face(Character *from, Direction direction);
		void Sit(Character *from, SitAction sit_type);
		void Stand(Character *from);
		void Emote(Character *from, enum Emote emote, bool echo = true);
		bool OpenDoor(Character *from, unsigned char x, unsigned char y);

		bool Walk(NPC *from, Direction direction);

		Map_Item *AddItem(short id, int amount, unsigned char x, unsigned char y, Character *from = 0);
		Map_Item *GetItem(short uid);
		void DelItem(short uid, Character *from = 0);
		void DelItem(Map_Item *item, Character *from = 0);

		bool InBounds(unsigned char x, unsigned char y);
		bool Walkable(unsigned char x, unsigned char y, bool npc = false);
		Map_Tile::TileSpec GetSpec(unsigned char x, unsigned char y);
		Map_Warp *GetWarp(unsigned char x, unsigned char y);

		void Effect(int effect, int param);

		bool Reload();

		Character *GetCharacter(std::string name);
		Character *GetCharacterPID(unsigned int id);
		Character *GetCharacterCID(unsigned int id);

		enum OccupiedTarget
		{
			PlayerOnly,
			NPCOnly,
			PlayerAndNPC
		};
		bool Occupied(unsigned char x, unsigned char y, Map::OccupiedTarget target);

		~Map();

	Map *ScriptFactory(int id, World *server) { return new Map(id, server); }

	SCRIPT_REGISTER_REF(Map)
		//SCRIPT_REGISTER_FACTORY("void f(int id, World @server)", ScriptFactory);

		SCRIPT_REGISTER_ENUM("OccupiedTarget");
			SCRIPT_REGISTER_ENUM_VALUE("OccupiedTarget", PlayerOnly);
			SCRIPT_REGISTER_ENUM_VALUE("OccupiedTarget", NPCOnly);
			SCRIPT_REGISTER_ENUM_VALUE("OccupiedTarget", PlayerAndNPC);

		SCRIPT_REGISTER_VARIABLE("World @", " world", world);
		SCRIPT_REGISTER_VARIABLE("int16", "id", id);
		//SCRIPT_REGISTER_VARIABLE("int8", "rid[4]", rid);
		SCRIPT_REGISTER_VARIABLE("bool", "pk", pk);
		SCRIPT_REGISTER_VARIABLE("int", "filesize", filesize);
		SCRIPT_REGISTER_VARIABLE("uint8", "width", width);
		SCRIPT_REGISTER_VARIABLE("uint8", "height", height);
		SCRIPT_REGISTER_VARIABLE("bool", "scroll", scroll);
		SCRIPT_REGISTER_VARIABLE("uint8", "relog_x", relog_x);
		SCRIPT_REGISTER_VARIABLE("uint8", "relog_y", relog_y);
		SCRIPT_REGISTER_VARIABLE("string", "filename", filename);
		SCRIPT_REGISTER_VARIABLE("PtrList<Character>", "characters", characters);
		SCRIPT_REGISTER_VARIABLE("PtrVector<NPC>", "npcs", npcs);
		SCRIPT_REGISTER_VARIABLE("PtrVector<Map_Chest>", "chests", chests);
		SCRIPT_REGISTER_VARIABLE("PtrList<Map_Item>", "items", items);
		SCRIPT_REGISTER_VARIABLE("PtrVector<PtrVector<Map_Tile>>", "tiles", tiles);
		SCRIPT_REGISTER_VARIABLE("bool", "exists", exists);
		SCRIPT_REGISTER_VARIABLE("double", "jukebox_protect", jukebox_protect);
		SCRIPT_REGISTER_VARIABLE("string", "jukebox_player", jukebox_player);
		SCRIPT_REGISTER_VARIABLE("Arena @", " arena", arena);
		SCRIPT_REGISTER_FUNCTION("int GenerateItemID()", GenerateItemID);
		SCRIPT_REGISTER_FUNCTION("uint8 GenerateNPCIndex()", GenerateNPCIndex);
		SCRIPT_REGISTER_FUNCTION("void Enter(Character @, WarpAnimation animation)", Enter);
		SCRIPT_REGISTER_FUNCTION("void Leave(Character @, WarpAnimation animation, bool silent)", Leave);
		SCRIPT_REGISTER_FUNCTION("void Msg(Character @from, string message, bool echo)", Msg);
		SCRIPT_REGISTER_FUNCTION_PR("bool Walk(Character @from, Direction direction, bool admin)", Walk, (Character *, Direction, bool), bool);
		SCRIPT_REGISTER_FUNCTION("void Attack(Character @from, Direction direction)", Attack);
		SCRIPT_REGISTER_FUNCTION("bool AttackPK(Character @from, Direction direction)", AttackPK);
		SCRIPT_REGISTER_FUNCTION("void Face(Character @from, Direction direction)", Face);
		SCRIPT_REGISTER_FUNCTION("void Sit(Character @from, SitAction sit_type)", Sit);
		SCRIPT_REGISTER_FUNCTION("void Stand(Character @from)", Stand);
		SCRIPT_REGISTER_FUNCTION("void Emote(Character @from, Emote emote, bool echo)", Emote);
		SCRIPT_REGISTER_FUNCTION("bool OpenDoor(Character @from, uint8 x, uint8 y)", OpenDoor);
		SCRIPT_REGISTER_FUNCTION_PR("bool Walk(NPC @from, Direction direction)", Walk, (NPC *, Direction), bool);
		SCRIPT_REGISTER_FUNCTION("Map_Item @AddItem(int16 id, int amount, uint8 x, uint8 y, Character @from)", AddItem);
		SCRIPT_REGISTER_FUNCTION("void GetItem(int16 uid)", GetItem);
		SCRIPT_REGISTER_FUNCTION_PR("void DelItem(int16 uid, Character @from)", DelItem, (short, Character *), void);
		SCRIPT_REGISTER_FUNCTION_PR("void DelItem(Map_Item @item, Character @from)", DelItem, (Map_Item *, Character *), void);
		SCRIPT_REGISTER_FUNCTION("bool InBounds(uint8 x, uint8 y)", InBounds);
		SCRIPT_REGISTER_FUNCTION("bool Walkable(uint8 x, uint8 y, bool npc)", Walkable);
		SCRIPT_REGISTER_FUNCTION("TileSpec GetSpec(uint8 x, uint8 y)", GetSpec);
		SCRIPT_REGISTER_FUNCTION("Map_Warp @GetWarp(uint8 x, uint8 y)", GetWarp);
		SCRIPT_REGISTER_FUNCTION("void Effect(int effect, int param)", Effect);
		SCRIPT_REGISTER_FUNCTION("bool Reload()", Reload);
		SCRIPT_REGISTER_FUNCTION("Character @GetCharacter(string name)", GetCharacter);
		SCRIPT_REGISTER_FUNCTION("Character @GetCharacterPID(uint id)", GetCharacterPID);
		SCRIPT_REGISTER_FUNCTION("Character @GetCharacterCID(uint id)", GetCharacterCID);
		SCRIPT_REGISTER_FUNCTION("bool Occupied(uint8 x, uint8 y, OccupiedTarget target)", Occupied);
	SCRIPT_REGISTER_END()
};

#endif // MAP_HPP_INCLUDED
