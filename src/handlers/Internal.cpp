
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

#include "map.hpp"

CLIENT_F_FUNC(Internal)
{
	if (!act)
	{
		Console::Wrn("Bad internal call");
		return false;
	}

	switch (action)
	{
		case PACKET_INTERNAL_NULL:
			break;

		case PACKET_INTERNAL_WARP: // Death warp
		{
			this->player->character->map = 0;
			this->player->character->Warp(this->player->character->SpawnMap(), this->player->character->SpawnX(), this->player->character->SpawnY(), WARP_ANIMATION_NONE);
		}
		break;


		default:
			return false;
	}

	return true;
}
