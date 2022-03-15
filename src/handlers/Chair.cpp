/* handlers/Chair.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../map.hpp"
#include "../packet.hpp"

#include "../util.hpp"

namespace Handlers
{

// Player sitting on a chair
void Chair_Request(Character *character, PacketReader &reader)
{
	int action = reader.GetChar();

	if (action == SIT_ACT_SIT && character->sitting == SIT_STAND)
	{
		int x = reader.GetChar();
		int y = reader.GetChar();

		if ((x + y - character->x - character->y) > 1)
		{
			return;
		}

		UTIL_FOREACH(character->map->characters, c)
		{
			if (c->x == x && c->y == y)
			{
				return;
			}
		}

		switch (character->map->GetSpec(x, y))
		{
			case Map_Tile::ChairDown:
				if (character->y == y+1 && character->x == x)
				{
					character->direction = DIRECTION_DOWN;
				}
				else
				{
					return;
				}
				break;

			case Map_Tile::ChairUp:
				if (character->y == y-1 && character->x == x)
				{
					character->direction = DIRECTION_UP;
				}
				else
				{
					return;
				}
				break;

			case Map_Tile::ChairLeft:
				if (character->y == y && character->x == x-1)
				{
					character->direction = DIRECTION_LEFT;
				}
				else
				{
					return;
				}
				break;

			case Map_Tile::ChairRight:
				if (character->y == y && character->x == x+1)
				{
					character->direction = DIRECTION_RIGHT;
				}
				else
				{
					return;
				}
				break;

			case Map_Tile::ChairDownRight:
				if (character->y == y && character->x == x+1)
				{
					character->direction = DIRECTION_RIGHT;
				}
				else if (character->y == y+1 && character->x == x)
				{
					character->direction = DIRECTION_DOWN;
				}
				else
				{
					return;
				}
				break;

			case Map_Tile::ChairUpLeft:
				if (character->y == y && character->x == x-1)
				{
					character->direction = DIRECTION_LEFT;
				}
				else if (character->y == y-1 && character->x == x)
				{
					character->direction = DIRECTION_UP;
				}
				else
				{
					return;
				}
				break;

			case Map_Tile::ChairAll:
				if (character->y == y && character->x == x+1)
				{
					character->direction = DIRECTION_RIGHT;
				}
				else if (character->y == y && character->x == x-1)
				{
					character->direction = DIRECTION_LEFT;
				}
				else if (character->y == y-1 && character->x == x)
				{
					character->direction = DIRECTION_UP;
				}
				else if (character->y == y+1 && character->x == x)
				{
					character->direction = DIRECTION_DOWN;
				}
				else
				{
					return;
				}
				break;

			default:
				return;
		}

		character->x = x;
		character->y = y;

		PacketBuilder reply(PACKET_CHAIR, PACKET_PLAYER, 6);
		reply.AddShort(character->PlayerID());
		reply.AddChar(character->x);
		reply.AddChar(character->y);
		reply.AddChar(character->direction);
		reply.AddChar(0); // ?
		character->Send(reply);
		character->Sit(SIT_CHAIR);
	}
	else if (character->sitting == SIT_CHAIR)
	{
		switch (character->direction)
		{
			case DIRECTION_UP:
				--character->y;
				break;
			case DIRECTION_RIGHT:
				++character->x;
				break;
			case DIRECTION_DOWN:
				++character->y;
				break;
			case DIRECTION_LEFT:
				--character->x;
				break;
		}

		PacketBuilder reply(PACKET_CHAIR, PACKET_CLOSE, 4);
		reply.AddShort(character->PlayerID());
		reply.AddChar(character->x);
		reply.AddChar(character->y);
		character->Send(reply);
		character->Stand();
	}
}

PACKET_HANDLER_REGISTER(PACKET_CHAIR)
	Register(PACKET_REQUEST, Chair_Request, Playing);
PACKET_HANDLER_REGISTER_END(PACKET_CHAIR)

}
