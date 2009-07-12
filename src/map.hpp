
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef MAP_HPP_INCLUDED
#define MAP_HPP_INCLUDED

#include <string>
#include <vector>

class Map;

struct Map_Item;
struct Map_Warp;
struct Map_Tile;

#include "world.hpp"
#include "eoconst.hpp"
#include "character.hpp"
#include "npc.hpp"
#include "arena.hpp"

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

	Map_Warp() : spec(Map_Warp::NoDoor), open(false) {}
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

	Map_Warp *warp;

	Map_Tile() : tilespec(Map_Tile::None), warp(0) {}

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

	~Map_Tile()
	{
		if (this->warp)
		{
			delete this->warp;
		}
	}
};

/**
 * Contains all information about a map, holds reference to contained Characters and manages NPCs on it
 */
class Map
{
	private:
		void Load();
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
		std::vector<Character *> characters;
		std::vector<NPC *> npcs;
		std::vector<Map_Item> items;
		std::vector<std::vector<Map_Tile> > tiles;
		bool exists;

		Arena *arena;

		Map(int id, World *server);

		int GenerateItemID();
		unsigned char GenerateNPCIndex();

		void Enter(Character *, WarpAnimation animation = WARP_ANIMATION_NONE);
		void Leave(Character *, WarpAnimation animation = WARP_ANIMATION_NONE);

		void Msg(Character *from, std::string message);
		bool Walk(Character *from, Direction direction, bool admin = false);
		void Attack(Character *from, Direction direction);
		void Face(Character *from, Direction direction);
		void Sit(Character *from, SitAction sit_type);
		void Stand(Character *from);
		void Emote(Character *from, enum Emote emote, bool relay = true);
		bool OpenDoor(Character *from, unsigned char x, unsigned char y);

		bool Walk(NPC *from, Direction direction);

		Map_Item *AddItem(short id, int amount, unsigned char x, unsigned char y, Character *from = 0);
		void DelItem(short uid, Character *from = 0);

		bool InBounds(unsigned char x, unsigned char y);
		bool Walkable(unsigned char x, unsigned char y, bool npc = false);
		Map_Tile::TileSpec GetSpec(unsigned char x, unsigned char y);
		Map_Warp *GetWarp(unsigned char x, unsigned char y);

		void Effect(int effect, int param);

		void Reload();

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
};

#endif // MAP_HPP_INCLUDED
