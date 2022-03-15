/* arena.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef ARENA_HPP_INCLUDED
#define ARENA_HPP_INCLUDED

#include "fwd/arena.hpp"

#include "fwd/character.hpp"
#include "fwd/map.hpp"
#include "fwd/timer.hpp"

#include <vector>

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

		TimeEvent *spawn_timer;
		Map *map;
		std::vector<Arena_Spawn *> spawns;

		Arena(Map *map, int time, int block);

		void Spawn(bool force = false);

		void Attack(Character *from, Direction);

		~Arena();
};

#endif // ARENA_HPP_INCLUDED
