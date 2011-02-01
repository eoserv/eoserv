
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef ARENA_HPP_INCLUDED
#define ARENA_HPP_INCLUDED

#include "fwd/arena.hpp"

#include "fwd/timer.hpp"
#include "container/ptr_vector.hpp"
#include "script.hpp"
#include "shared.hpp"

#include "fwd/character.hpp"
#include "fwd/map.hpp"

struct Arena_Spawn : public Shared
{
	unsigned char sx;
	unsigned char sy;
	unsigned char dx;
	unsigned char dy;

	SCRIPT_REGISTER_REF_DF(Arena_Spawn)
		SCRIPT_REGISTER_VARIABLE("uint8", sx);
		SCRIPT_REGISTER_VARIABLE("uint8", sy);
		SCRIPT_REGISTER_VARIABLE("uint8", dx);
		SCRIPT_REGISTER_VARIABLE("uint8", dy);
	SCRIPT_REGISTER_END()
};

class Arena : public Shared
{
	public:
		int occupants;
		int time;
		int block;

		TimeEvent *spawn_timer;
		Map *map;
		PtrVector<Arena_Spawn> spawns;

		Arena(Map *map, int time, int block);

		void Spawn(bool force = false);

		void Attack(Character *from, Direction);

		~Arena();

	SCRIPT_REGISTER_REF(Arena)
		SCRIPT_REGISTER_VARIABLE("int", occupants);
		SCRIPT_REGISTER_VARIABLE("int", time);
		SCRIPT_REGISTER_VARIABLE("int", block);
		SCRIPT_REGISTER_VARIABLE("TimeEvent @", spawn_timer);
		SCRIPT_REGISTER_VARIABLE("Map @", map);
		SCRIPT_REGISTER_VARIABLE("PtrVector<Arena_Spawn>", spawns);
		SCRIPT_REGISTER_FUNCTION("void Spawn(bool force)", Spawn);
		SCRIPT_REGISTER_FUNCTION("void Attack(Character @from, Direction)", Spawn);
	SCRIPT_REGISTER_END()
};

#endif // ARENA_HPP_INCLUDED
