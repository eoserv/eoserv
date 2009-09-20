
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

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

		case PACKET_INTERNAL_WARP: // Warp
		{
			short map = reader.GetShort();
			unsigned char x = reader.GetChar();
			unsigned char y = reader.GetChar();

			this->player->character->Warp(map, x, y);
		}
		break;


		default:
			return false;
	}

	return true;
}
