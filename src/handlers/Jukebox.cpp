
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

#include "map.hpp"

CLIENT_F_FUNC(Jukebox)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_OPEN: // Opened the jukebox listing
		{
			if (this->state < EOClient::PlayingModal) return false;

			unsigned char x = reader.GetChar();
			unsigned char y = reader.GetChar();

			if (!this->player->character->InRange(x, y)
			 || this->player->character->map->GetSpec(x, y) != Map_Tile::Jukebox)
			{
				return true;
			}

			reply.SetID(PACKET_JUKEBOX, PACKET_OPEN);
			reply.AddShort(this->player->character->mapid);

			if (this->player->character->map->jukebox_protect > Timer::GetTime())
			{
				reply.AddString(this->player->character->map->jukebox_player);
			}

			this->player->character->jukebox_open = true;

			CLIENT_SEND(reply);
		}
		break;

		case PACKET_MSG: // Requested a song
		{
			if (this->state < EOClient::PlayingModal) return false;

			reader.GetChar();
			reader.GetChar();
			short track = reader.GetShort();

			if (!this->player->character->jukebox_open
			 || this->player->character->map->jukebox_protect > Timer::GetTime()
			 || (track < 0 || track > static_cast<int>(this->server->world->config["JukeboxSongs"]))
			 || this->player->character->HasItem(1) < static_cast<int>(this->server->world->config["JukeboxPrice"]))
			{
				return true;
			}

			this->player->character->DelItem(1, static_cast<int>(this->server->world->config["JukeboxPrice"]));

			this->player->character->map->jukebox_player = this->player->character->name;
			this->player->character->map->jukebox_protect = Timer::GetTime() + static_cast<int>(this->server->world->config["JukeboxTimer"]);

			reply.SetID(PACKET_JUKEBOX, PACKET_AGREE);
			reply.AddInt(this->player->character->HasItem(1));

			CLIENT_SEND(reply);

			PacketBuilder builder(PACKET_JUKEBOX, PACKET_USE);
			builder.AddShort(track + 1);
			UTIL_PTR_LIST_FOREACH(this->player->character->map->characters, Character, character)
			{
				character->player->client->SendBuilder(builder);
			}
		}
		break;

		default:
			return false;
	}

	return true;
}
