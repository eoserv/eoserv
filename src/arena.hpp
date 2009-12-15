
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef ARENA_HPP_INCLUDED
#define ARENA_HPP_INCLUDED

#include "stdafx.h"

struct Arena_Spawn : public Shared
{
	unsigned char sx;
	unsigned char sy;
	unsigned char dx;
	unsigned char dy;

	SCRIPT_REGISTER_REF_DF(Arena_Spawn)
		SCRIPT_REGISTER_VARIABLE("uint8", "sx", sx);
		SCRIPT_REGISTER_VARIABLE("uint8", "sy", sy);
		SCRIPT_REGISTER_VARIABLE("uint8", "dx", dx);
		SCRIPT_REGISTER_VARIABLE("uint8", "dy", sy);
	SCRIPT_REGISTER_END()
};

class Arena : public Shared
{
	public:
		int occupants;
		int time;
		int block;

		Timer *spawn_timer;
		Map *map;
		PtrVector<Arena_Spawn> spawns;

		Arena(Map *map, int time, int block);

		void Spawn(bool force = false);

		void Attack(Character *from, Direction);

	SCRIPT_REGISTER_REF(Arena)
		SCRIPT_REGISTER_VARIABLE("int", "occupants", occupants);
		SCRIPT_REGISTER_VARIABLE("int", "time", time);
		SCRIPT_REGISTER_VARIABLE("int", "block", block);
		SCRIPT_REGISTER_VARIABLE("Timer @", "spawn_timer", block);
		SCRIPT_REGISTER_VARIABLE("Map @", "map", map);
		SCRIPT_REGISTER_VARIABLE("PtrVector<Arena_Spawn>", "spawns", spawns);
		SCRIPT_REGISTER_FUNCTION("void Spawn(bool force)", Spawn);
		SCRIPT_REGISTER_FUNCTION("void Attack(Character @from, Direction)", Spawn);
	SCRIPT_REGISTER_END()
};

#endif // ARENA_HPP_INCLUDED
