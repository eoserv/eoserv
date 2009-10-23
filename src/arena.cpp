
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "arena.hpp"

#include "character.hpp"
#include "console.hpp"
#include "eoclient.hpp"
#include "map.hpp"
#include "packet.hpp"
#include "player.hpp"
#include "timer.hpp"
#include "world.hpp"

void arena_spawn(void *arena_void)
{
	Arena *arena = static_cast<Arena *>(arena_void);

	arena->Spawn();
}

Arena::Arena(Map *map, int time, int block)
{
	this->map = map;
	this->time = time;
	this->block = block;
	this->occupants = 0;

	this->map->world->timer.Register(new TimeEvent(arena_spawn, this, time, Timer::FOREVER, true));
}

void Arena::Spawn(bool force)
{
	int newplayers = 0;

	UTIL_VECTOR_FOREACH_ALL(this->spawns, Arena_Spawn, spawn)
	{
		UTIL_LIST_FOREACH_ALL(this->map->characters, Character *, character)
		{
			if (character->x == spawn.sx && character->y == spawn.sy)
			{
				++newplayers;
			}
		}
	}

	if (newplayers == 0)
	{
		return;
	}

	if (this->occupants >= this->block && !force)
	{
		PacketBuilder builder(PACKET_ARENA, PACKET_DROP);

		UTIL_LIST_FOREACH_ALL(this->map->characters, Character *, character)
		{
			character->player->client->SendBuilder(builder);
		}

		return;
	}

	UTIL_VECTOR_FOREACH_ALL(this->spawns, Arena_Spawn, spawn)
	{
		UTIL_LIST_FOREACH_ALL(this->map->characters, Character *, character)
		{
			if (character->x == spawn.sx && character->y == spawn.sy)
			{
				character->next_arena = this;
				character->arena_kills = 0;
				character->Warp(this->map->id, spawn.dx, spawn.dy);
				break;
			}
		}
	}

	PacketBuilder builder(PACKET_ARENA, PACKET_USE);
	builder.AddChar(newplayers);

	UTIL_LIST_FOREACH_ALL(this->map->characters, Character *, character)
	{
		character->player->client->SendBuilder(builder);
	}
}

void Arena::Attack(Character *from, Direction direction)
{
	int target_x = from->x;
	int target_y = from->y;

	switch (from->direction)
	{
		case DIRECTION_UP:
			target_y -= 1;
			break;

		case DIRECTION_RIGHT:
			target_x += 1;
			break;

		case DIRECTION_DOWN:
			target_y += 1;
			break;

		case DIRECTION_LEFT:
			target_x -= 1;
			break;
	}

	UTIL_LIST_FOREACH_ALL(this->map->characters, Character *, character)
	{
		if (character->arena == this && character->x == target_x && character->y == target_y)
		{
			++from->arena_kills;
			character->Warp(this->map->id, this->map->relog_x, this->map->relog_y);

			PacketBuilder builder(PACKET_ARENA, PACKET_SPEC);
			builder.AddShort(0); // ?
			builder.AddByte(255);
			builder.AddChar(0); // ?
			builder.AddByte(255);
			builder.AddShort(from->arena_kills);
			builder.AddChar(0); // ?
			builder.AddByte(255);
			builder.AddBreakString(from->name);
			builder.AddBreakString(character->name);

			UTIL_LIST_FOREACH_ALL(this->map->characters, Character *, character)
			{
				character->player->client->SendBuilder(builder);
			}

			if (from->arena->occupants == 1)
			{
				from->Warp(this->map->id, this->map->relog_x, this->map->relog_y);

				builder.Reset();
				builder.SetID(PACKET_ARENA, PACKET_ACCEPT);
				builder.AddBreakString(from->name);

				UTIL_LIST_FOREACH_ALL(this->map->characters, Character *, character)
				{
					character->player->client->SendBuilder(builder);
				}
			}

			break;
		}
	}
}
