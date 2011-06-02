
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include <algorithm>
#include <functional>

#include "character.hpp"
#include "map.hpp"
#include "player.hpp"

namespace Handlers
{

// Opened the jukebox listing
void Jukebox_Open(Character *character, PacketReader &reader)
{
	unsigned char x = reader.GetChar();
	unsigned char y = reader.GetChar();

	if (!character->InRange(x, y)
	 || character->map->GetSpec(x, y) != Map_Tile::Jukebox)
	{
		return;
	}

	PacketBuilder reply(PACKET_JUKEBOX, PACKET_OPEN, 14);
	reply.AddShort(character->mapid);

	if (character->map->jukebox_protect > Timer::GetTime())
	{
		reply.AddString(character->map->jukebox_player);
	}

	character->jukebox_open = true;

	character->Send(reply);
}

// Requested a song
void Jukebox_Msg(Character *character, PacketReader &reader)
{
	if (character->trading) return;

	using namespace std::placeholders;

	reader.GetChar();
	reader.GetChar();
	short track = reader.GetShort();

	if (!character->jukebox_open
	 || character->map->jukebox_protect > Timer::GetTime()
	 || (track < 0 || track > static_cast<int>(character->world->config["JukeboxSongs"]))
	 || character->HasItem(1) < static_cast<int>(character->world->config["JukeboxPrice"]))
	{
		return;
	}

	character->DelItem(1, static_cast<int>(character->world->config["JukeboxPrice"]));

	character->map->jukebox_player = character->name;
	character->map->jukebox_protect = Timer::GetTime() + static_cast<int>(character->world->config["JukeboxTimer"]);

	PacketBuilder reply(PACKET_JUKEBOX, PACKET_AGREE, 4);
	reply.AddInt(character->HasItem(1));
	character->Send(reply);

	PacketBuilder builder(PACKET_JUKEBOX, PACKET_USE, 2);
	builder.AddShort(track + 1);
	std::for_each(UTIL_CRANGE(character->map->characters), std::bind(&Character::Send, _1, builder));
}

PACKET_HANDLER_REGISTER(PACKET_JUKEBOX)
	Register(PACKET_OPEN, Jukebox_Open, Playing);
	Register(PACKET_MSG, Jukebox_Msg, Playing);
PACKET_HANDLER_REGISTER_END()

}
