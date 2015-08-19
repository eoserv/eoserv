
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef MAP_HPP_INCLUDED
#define MAP_HPP_INCLUDED

#include "fwd/map.hpp"

#include <list>
#include <memory>
#include <string>
#include <vector>

#include "fwd/arena.hpp"
#include "fwd/character.hpp"
#include "fwd/npc.hpp"
#include "fwd/world.hpp"

/**
 * Object representing an item on the floor of a map
 */
struct Map_Item
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
};

/**
 * Object representing a warp tile on a map, as well as storing door state
 */
struct Map_Warp
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

	Map_Warp() : map(0), x(0), y(0), levelreq(0), spec(Map_Warp::NoDoor), open(false) {}

	explicit operator bool() const
	{
		return map != 0;
	}
};

/**
 * Object representing one tile on a map
 */
struct Map_Tile
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

	Map_Warp warp;

	Map_Tile() : tilespec(Map_Tile::None) { }

	bool Walkable(bool npc = false) const
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
			case Board8:
			case Jukebox:
				return false;
			case NPCBoundary:
				return !npc;
			default:
				return true;
		}
	}
};

/**
 * Object representing an item in a chest on a map
 */
struct Map_Chest_Item
{
	short id;
	int amount;
	int slot;
};

/**
 * Object representing an item spawn in a chest on a map
 */
struct Map_Chest_Spawn
{
	int spawnid;
	Map_Chest_Item item;
	int slot;
	short time;
	double last_taken;
};

/**
 * Object representing a chest on a map
 */
struct Map_Chest
{
	unsigned char x;
	unsigned char y;

	std::list<Map_Chest_Item> items;
	std::list<Map_Chest_Spawn> spawns;
	int slots;

	int maxchest;
	int chestslots;

	int HasItem(short item) const;
	int AddItem(short item, int amount, int slot = 0);
	int DelItem(short item);
	int DelSomeItem(short item, int amount);

	void Update(Map *map, Character *exclude = 0) const;
};

/**
 * Contains all information about a map, holds reference to contained Characters and manages NPCs on it
 */
class Map
{
	private:
		bool Load();
		void Unload();

	public:
		enum WalkResult
		{
			WalkFail = 0,
			WalkOK = 1,
			WalkWarped = 2
		};

		enum EffectType
		{
			EffectNone = 0,
			EffectHPDrain = 1,
			EffectTPDrain = 2,
			EffectQuake1 = 3,
			EffectQuake2 = 4,
			EffectQuake3 = 5,
			EffectQuake4 = 6
		};

		World *world;
		short id;
		char rid[4];
		bool pk;
		EffectType effect;
		int filesize;
		unsigned char width;
		unsigned char height;
		bool scroll;
		unsigned char relog_x;
		unsigned char relog_y;
		std::list<Character *> characters;
		std::vector<NPC *> npcs;
		std::vector<std::shared_ptr<Map_Chest>> chests;
		std::list<std::shared_ptr<Map_Item>> items;
		std::vector<Map_Tile> tiles;
		bool exists;
		double jukebox_protect;
		std::string jukebox_player;
		bool evacuate_lock;
		bool has_timed_spikes;

		Arena *arena;

		Map(int id, World *world);
		void LoadArena();

		int GenerateItemID() const;
		unsigned char GenerateNPCIndex() const;

		void Enter(Character *, WarpAnimation animation = WARP_ANIMATION_NONE);
		void Leave(Character *, WarpAnimation animation = WARP_ANIMATION_NONE, bool silent = false);

		void Msg(Character *from, std::string message, bool echo = true);
		void Msg(NPC *from, std::string message);
		WalkResult Walk(Character *from, Direction direction, bool admin = false);
		void Attack(Character *from, Direction direction);
		bool AttackPK(Character *from, Direction direction);
		void Face(Character *from, Direction direction);
		void Sit(Character *from, SitState sit_type);
		void Stand(Character *from);
		void Emote(Character *from, enum Emote emote, bool echo = true);
		bool OpenDoor(Character *from, unsigned char x, unsigned char y);
		void CloseDoor(unsigned char x, unsigned char y);

		void SpellSelf(Character *from, unsigned short spell_id);
		void SpellAttack(Character *from, NPC *victim, unsigned short spell_id);
		void SpellAttackPK(Character *from, Character *victim, unsigned short spell_id);
		void SpellGroup(Character *from, unsigned short spell_id);

		WalkResult Walk(NPC *from, Direction direction);

		std::shared_ptr<Map_Item> AddItem(short id, int amount, unsigned char x, unsigned char y, Character *from = 0);

		std::shared_ptr<Map_Item> GetItem(short uid);
		std::shared_ptr<const Map_Item> GetItem(short uid) const;

		void DelItem(short uid, Character *from = 0);
		void DelSomeItem(short uid, int amount, Character *from = 0);
		std::list<std::shared_ptr<Map_Item>>::iterator DelItem(std::list<std::shared_ptr<Map_Item>>::iterator it, Character *from = 0);

		bool InBounds(unsigned char x, unsigned char y) const;
		bool Walkable(unsigned char x, unsigned char y, bool npc = false) const;
		Map_Tile& GetTile(unsigned char x, unsigned char y);
		const Map_Tile& GetTile(unsigned char x, unsigned char y) const;
		Map_Tile::TileSpec GetSpec(unsigned char x, unsigned char y) const;
		Map_Warp& GetWarp(unsigned char x, unsigned char y);
		const Map_Warp& GetWarp(unsigned char x, unsigned char y) const;

		std::vector<Character *> CharactersInRange(unsigned char x, unsigned char y, unsigned char range);
		std::vector<NPC *> NPCsInRange(unsigned char x, unsigned char y, unsigned char range);

		void Effect(MapEffect effect, unsigned char param);

		bool Evacuate();

		bool Reload();

		void TimedSpikes();
		void TimedDrains();
		void TimedQuakes();

		Character *GetCharacter(std::string name);
		Character *GetCharacterPID(unsigned int id);
		Character *GetCharacterCID(unsigned int id);
		NPC *GetNPCIndex(unsigned char index);

		enum OccupiedTarget
		{
			PlayerOnly,
			NPCOnly,
			PlayerAndNPC
		};
		bool Occupied(unsigned char x, unsigned char y, Map::OccupiedTarget target) const;

		~Map();
};

#endif // MAP_HPP_INCLUDED
