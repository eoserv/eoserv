/* arena.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "arena.hpp"

#include "character.hpp"
#include "eoclient.hpp"
#include "map.hpp"
#include "packet.hpp"
#include "timer.hpp"
#include "world.hpp"

#include "console.hpp"
#include "util.hpp"

#include <vector>

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

	this->spawn_timer = new TimeEvent(arena_spawn, this, time, Timer::FOREVER);
	this->map->world->timer.Register(this->spawn_timer);
}

void Arena::Spawn(bool force)
{
	int newplayers = 0;

	UTIL_FOREACH(this->spawns, spawn)
	{
		UTIL_FOREACH(this->map->characters, character)
		{
			if (character->x == spawn->sx && character->y == spawn->sy)
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

		UTIL_FOREACH(this->map->characters, character)
		{
			character->Send(builder);
		}

		return;
	}

	struct Arena_Spawn_Action
	{
		Character *character;
		short map;
		unsigned char x;
		unsigned char y;

		Arena_Spawn_Action(Character *character, short map, unsigned char x, unsigned char y)
			: character(character)
			, map(map)
			, x(x)
			, y(y)
		{ }
	};

	std::vector<Arena_Spawn_Action> actions;

	UTIL_FOREACH(this->spawns, spawn)
	{
		UTIL_FOREACH(this->map->characters, character)
		{
			if (character->x == spawn->sx && character->y == spawn->sy)
			{
				character->next_arena = this;
				character->arena_kills = 0;
				actions.push_back({character, this->map->id, spawn->dx, spawn->dy});
				break;
			}
		}
	}

	UTIL_FOREACH(actions, act)
	{
		act.character->Warp(act.map, act.x, act.y);
	}

	PacketBuilder builder(PACKET_ARENA, PACKET_USE, 1);
	builder.AddChar(newplayers);

	UTIL_FOREACH(this->map->characters, character)
	{
		character->Send(builder);
	}
}

void Arena::Attack(Character *from, Direction direction)
{
	(void)direction;

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

	struct Arena_Spawn_Action
	{
		Character *character;
		short map;
		unsigned char x;
		unsigned char y;

		Arena_Spawn_Action(Character *character, short map, unsigned char x, unsigned char y)
			: character(character)
			, map(map)
			, x(x)
			, y(y)
		{ }
	};

	std::vector<Arena_Spawn_Action> actions;

	UTIL_FOREACH(this->map->characters, character)
	{
		if (character->arena == this && character->x == target_x && character->y == target_y)
		{
			++from->arena_kills;
			actions.push_back({character, this->map->id, this->map->relog_x, this->map->relog_y});

			// TODO: The numbers here seem to be variable-sized

			PacketBuilder builder(PACKET_ARENA, PACKET_SPEC, 12 + from->SourceName().length() + character->SourceName().length());
			builder.AddShort(0); // ?
			builder.AddByte(255);
			builder.AddChar(0); // ?
			builder.AddByte(255);
			builder.AddInt(from->arena_kills);
			builder.AddByte(255);
			builder.AddBreakString(from->SourceName());
			builder.AddBreakString(character->SourceName());

			if (from->arena->occupants == 2)
			{
				actions.push_back({from, this->map->id, this->map->relog_x, this->map->relog_y});

				PacketBuilder builder(PACKET_ARENA, PACKET_ACCEPT, 8 + from->SourceName().length() * 2 + character->SourceName().length());
				builder.AddBreakString(from->SourceName());
				builder.AddInt(from->arena_kills);
				builder.AddByte(255);
				builder.AddBreakString(from->SourceName());
				builder.AddBreakString(character->SourceName());

				UTIL_FOREACH(this->map->characters, character)
				{
					character->Send(builder);
				}
			}
			else
			{
				UTIL_FOREACH(this->map->characters, character)
				{
					character->Send(builder);
				}
			}

			break;
		}
	}

	UTIL_FOREACH(actions, act)
	{
		act.character->Warp(act.map, act.x, act.y);
	}
}

Arena::~Arena()
{
	delete this->spawn_timer;
}
