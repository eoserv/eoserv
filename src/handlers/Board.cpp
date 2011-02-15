
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

#include "util.hpp"

#include "character.hpp"
#include "map.hpp"
#include "player.hpp"
#include "world.hpp"

CLIENT_F_FUNC(Board)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REMOVE: // Removing a post from a town board
		{
			if (this->state < EOClient::Playing) return false;

			/*short boardid =*/ reader.GetShort();
			short postid = reader.GetShort();

			if (this->player->character->board)
			{
				if (this->player->character->admin < static_cast<int>(this->server()->world->admin_config["boardmod"]))
				{
					// Not in the official EO servers, but nice to use
					this->player->character->ShowBoard();
					return true;
				}

				UTIL_IFOREACH(this->player->character->board->posts, post)
				{
					if ((*post)->id == postid)
					{
						if ((*post)->author_admin < this->player->character->admin
						 || (*post)->author == this->player->character->name)
						{
							this->player->character->board->posts.erase(post);
						}
						break;
					}
				}

				// Not in the official EO servers, but nice to use
				this->player->character->ShowBoard();
			}
		}
		break;

		case PACKET_CREATE: // Posting to a message board
		{
			if (this->state < EOClient::Playing) return false;

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

			subject = subject.substr(0, static_cast<int>(this->server()->world->config["BoardMaxSubjectLength"]));
			body = body.substr(0, static_cast<int>(this->server()->world->config["BoardMaxPostLength"]));

			int post_count = 0;
			int recent_post_count = 0;

			if (this->player->character->board)
			{
				UTIL_FOREACH(this->player->character->board->posts, post)
				{
					if (post->author == this->player->character->name)
					{
						++post_count;

						if (post_count >= static_cast<int>(this->server()->world->config["BoardMaxUserPosts"]))
						{
							// Not in the official EO servers, but nice to use
							this->player->character->ShowBoard();
							return true;
						}

						if (post->time + static_cast<int>(this->server()->world->config["BoardRecentPostTime"]) > Timer::GetTime())
						{
							++recent_post_count;

							if (recent_post_count >= static_cast<int>(this->server()->world->config["BoardMaxUserRecentPosts"]))
							{
								// Not in the official EO servers, but nice to use
								this->player->character->ShowBoard();
								return true;
							}
						}
					}
				}

				Board_Post *newpost = new Board_Post;
				newpost->id = ++this->player->character->board->last_id;
				newpost->author = this->player->character->name;
				newpost->author_admin = this->player->character->admin;
				newpost->subject = subject;
				newpost->body = body;
				newpost->time = Timer::GetTime();

				this->player->character->board->posts.push_front(newpost);

				if (this->player->character->board->id == static_cast<int>(this->server()->world->config["AdminBoard"]))
				{
					if (this->player->character->board->posts.size() > static_cast<std::size_t>(static_cast<int>(this->server()->world->config["AdminBoardLimit"])))
					{
						this->player->character->board->posts.pop_back();
					}
				}
				else
				{
					if (this->player->character->board->posts.size() > static_cast<std::size_t>(static_cast<int>(this->server()->world->config["BoardMaxPosts"])))
					{
						this->player->character->board->posts.pop_back();
					}
				}

				// Not in the official EO servers, but nice to use
				this->player->character->ShowBoard();
			}
		}
		break;

		case PACKET_TAKE: // Reading a post on a town board
		{
			if (this->state < EOClient::Playing) return false;

			/*short boardid =*/ reader.GetShort();
			short postid = reader.GetShort();

			if (this->player->character->board)
			{
				UTIL_FOREACH(this->player->character->board->posts, post)
				{
					if (post->id == postid)
					{
						reply.SetID(PACKET_BOARD, PACKET_PLAYER);
						reply.AddShort(postid);
						reply.AddString(post->body);
						CLIENT_SEND(reply);
						break;
					}
				}
			}
		}
		break;

		case PACKET_OPEN: // Opening town board
		{
			if (this->state < EOClient::Playing) return false;
			CLIENT_QUEUE_ACTION(0.0)

			short boardid = reader.GetShort();

			if (static_cast<std::size_t>(boardid) >= this->server()->world->boards.size())
			{
				return true;
			}

			if (boardid != static_cast<int>(this->server()->world->config["AdminBoard"]) - 1
			 || this->player->character->admin >= static_cast<int>(this->server()->world->admin_config["reports"]))
			{
				for (std::size_t y = 0; y < this->player->character->map->height; ++y)
				{
					for (std::size_t x = 0; x < this->player->character->map->width; ++x)
					{
						if (this->player->character->InRange(x, y)
						 && this->player->character->map->GetSpec(x, y) == static_cast<Map_Tile::TileSpec>(Map_Tile::Board1 + boardid))
						{
							this->player->character->board = this->server()->world->boards[boardid];
						}
					}
				}
			}

			if (this->player->character->board)
			{
				this->player->character->ShowBoard();
			}
		}
		break;

		default:
			return false;
	}

	return true;
}
