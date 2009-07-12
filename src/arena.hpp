
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef ARENA_HPP_INCLUDED
#define ARENA_HPP_INCLUDED

class Arena;

struct Arena_Spawn;

#include "character.hpp"

#include "eoconst.hpp"
#include "timer.hpp"

struct Arena_Spawn
{
	unsigned char sx;
	unsigned char sy;
	unsigned char dx;
	unsigned char dy;
};

class Arena
{
	public:
		int occupants;
		int time;
		int block;

		Timer *spawn_timer;
		Map *map;
		std::vector<Arena_Spawn> spawns;

		Arena(Map *map, int time, int block);

		void Spawn(bool force = false);

		void Attack(Character *from, Direction);
};

#endif // ARENA_HPP_INCLUDED
