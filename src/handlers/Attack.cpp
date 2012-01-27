
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../map.hpp"
#include "../player.hpp"

namespace Handlers
{

// Player attacking
void Attack_Use(Character *character, PacketReader &reader)
{
	Direction direction = static_cast<Direction>(reader.GetChar());
	/*int timestamp = */reader.GetThree();

	if (character->sitting != SIT_STAND)
		return;

	if (int(character->world->config["EnforceWeight"]) >= 1 && character->weight > character->maxweight)
		return;

	int limit_attack = character->world->config["LimitAttack"];

	if (limit_attack != 0 && character->attacks >= limit_attack)
		return;

	// TODO: Find a way to implement this

	/*if (direction != character->direction)
	{
		if (direction >= 0 && direction <= 3)
		{
			character->map->Face(character, direction);
			//CLIENT_FORCE_QUEUE_ACTION(0.67)
		}
		else
		{
			return;
		}
	}*/

	direction = character->direction;

	character->Attack(direction);
}

PACKET_HANDLER_REGISTER(PACKET_ATTACK)
	Register(PACKET_USE, Attack_Use, Playing, 0.58);
PACKET_HANDLER_REGISTER_END()

}
