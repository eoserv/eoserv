/* handlers/Board.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../config.hpp"
#include "../map.hpp"
#include "../packet.hpp"
#include "../timer.hpp"
#include "../world.hpp"

#include "../util.hpp"

#include <cstddef>
#include <string>

namespace Handlers
{

// Removing a post from a town board
void Board_Remove(Character *character, PacketReader &reader)
{
	/*short boardid =*/ reader.GetShort();
	short postid = reader.GetShort();

	if (character->board)
	{
		if (character->SourceAccess() < static_cast<int>(character->world->admin_config["boardmod"]))
		{
			// Not in the official EO servers, but nice to use
			character->ShowBoard();
			return;
		}

		UTIL_IFOREACH(character->board->posts, post)
		{
			if ((*post)->id == postid)
			{
				if ((*post)->author_admin < character->admin
				 || (*post)->author == character->SourceName())
				{
					delete *post;
					character->board->posts.erase(post);
				}
				break;
			}
		}

		// Not in the official EO servers, but nice to use
		character->ShowBoard();
	}
}

// Posting to a message board
void Board_Create(Character *character, PacketReader &reader)
{
	/*short boardid =*/ reader.GetShort();
	reader.GetByte();
	std::string subject = reader.GetBreakString();
	std::string body = reader.GetBreakString();

	for (std::string::iterator i = subject.begin(); i != subject.end(); ++i)
	{
		if (static_cast<unsigned char>(*i) == 0xFF)
		{
			*i = 'y';
		}
	}

	subject = subject.substr(0, static_cast<int>(character->world->config["BoardMaxSubjectLength"]));
	body = body.substr(0, static_cast<int>(character->world->config["BoardMaxPostLength"]));

	int post_count = 0;
	int recent_post_count = 0;

	if (character->board)
	{
		UTIL_FOREACH(character->board->posts, post)
		{
			if (post->author == character->SourceName())
			{
				++post_count;

				if (post_count >= static_cast<int>(character->world->config["BoardMaxUserPosts"]))
				{
					// Not in the official EO servers, but nice to use
					character->ShowBoard();
					return;
				}

				if (post->time + static_cast<int>(character->world->config["BoardRecentPostTime"]) > Timer::GetTime())
				{
					++recent_post_count;

					if (recent_post_count >= static_cast<int>(character->world->config["BoardMaxUserRecentPosts"]))
					{
						// Not in the official EO servers, but nice to use
						character->ShowBoard();
						return;
					}
				}
			}
		}

		Board_Post *newpost = new Board_Post;
		newpost->id = ++character->board->last_id;
		newpost->author = character->SourceName();
		newpost->author_admin = character->admin;
		newpost->subject = subject;
		newpost->body = body;
		newpost->time = Timer::GetTime();

		character->board->posts.push_front(newpost);

		if (character->board->id == static_cast<int>(character->world->config["AdminBoard"]))
		{
			if (character->board->posts.size() > static_cast<std::size_t>(static_cast<int>(character->world->config["AdminBoardLimit"])))
			{
				character->board->posts.pop_back();
			}
		}
		else
		{
			if (character->board->posts.size() > static_cast<std::size_t>(static_cast<int>(character->world->config["BoardMaxPosts"])))
			{
				character->board->posts.pop_back();
			}
		}

		// Not in the official EO servers, but nice to use
		character->ShowBoard();
	}
}

// Reading a post on a town board
void Board_Take(Character *character, PacketReader &reader)
{
	/*short boardid =*/ reader.GetShort();
	short postid = reader.GetShort();

	if (character->board)
	{
		UTIL_FOREACH(character->board->posts, post)
		{
			if (post->id == postid)
			{
				PacketBuilder reply(PACKET_BOARD, PACKET_PLAYER, 2 + post->body.length());
				reply.AddShort(postid);
				reply.AddString(post->body);
				character->Send(reply);
				break;
			}
		}
	}
}

// Opening town board
void Board_Open(Character *character, PacketReader &reader)
{
	short boardid = reader.GetShort();

	if (static_cast<std::size_t>(boardid) >= character->world->boards.size())
	{
		return;
	}

	if (boardid != static_cast<int>(character->world->config["AdminBoard"]) - 1
	 || character->SourceAccess() >= static_cast<int>(character->world->admin_config["reports"]))
	{
		for (std::size_t y = 0; y < character->map->height; ++y)
		{
			for (std::size_t x = 0; x < character->map->width; ++x)
			{
				if (character->InRange(x, y)
				 && character->map->GetSpec(x, y) == static_cast<Map_Tile::TileSpec>(Map_Tile::Board1 + boardid))
				{
					character->board = character->world->boards[boardid];
				}
			}
		}
	}

	if (character->board)
	{
		character->ShowBoard();
	}
}

PACKET_HANDLER_REGISTER(PACKET_BOARD)
	Register(PACKET_REMOVE, Board_Remove, Playing);
	Register(PACKET_CREATE, Board_Create, Playing);
	Register(PACKET_TAKE, Board_Take, Playing);
	Register(PACKET_OPEN, Board_Open, Playing);
PACKET_HANDLER_REGISTER_END(PACKET_BOARD)

}
