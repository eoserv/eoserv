
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

CLIENT_F_FUNC(Board)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REMOVE: // Removing a post from a town board
		{
			if (this->state < EOClient::Playing) return false;

			short boardid = reader.GetShort();
			short postid = reader.GetShort();

			if (static_cast<std::size_t>(boardid) > this->server->world->boards.size())
			{
				return true;
			}

			if (this->player->character->admin < static_cast<int>(this->server->world->admin_config["boardmod"]))
			{
				// Not in the official EO servers, but nice to use
				this->player->character->ShowBoard(boardid);
				return true;
			}

			UTIL_PTR_LIST_FOREACH(this->server->world->boards[boardid]->posts, Board_Post, post)
			{
				if (post->id == postid)
				{
					if (post->author_admin < this->player->character->admin
					 || post->author == this->player->character->name)
					{
						this->server->world->boards[boardid]->posts.erase(post);
					}
					break;
				}
			}

			// Not in the official EO servers, but nice to use
			this->player->character->ShowBoard(boardid);
		}
		break;

		case PACKET_CREATE: // Posting to a message board
		{
			if (this->state < EOClient::Playing) return false;

			short boardid = reader.GetShort();
			reader.GetByte();
			std::string subject = reader.GetBreakString();
			std::string body = reader.GetBreakString();

			if (static_cast<std::size_t>(boardid) > this->server->world->boards.size())
			{
				return true;
			}

			for (std::string::iterator i = subject.begin(); i != subject.end(); ++i)
			{
				if (static_cast<unsigned char>(*i) == 0xFF)
				{
					*i = 'y';
				}
			}

			subject = subject.substr(0, static_cast<int>(this->server->world->config["BoardMaxSubjectLength"]));
			body = body.substr(0, static_cast<int>(this->server->world->config["BoardMaxPostLength"]));

			int post_count = 0;
			int recent_post_count = 0;

			UTIL_PTR_LIST_FOREACH(this->server->world->boards[boardid]->posts, Board_Post, post)
			{
				if (post->author == this->player->character->name)
				{
					++post_count;

					if (post_count >= static_cast<int>(this->server->world->config["BoardMaxUserPosts"]))
					{
						// Not in the official EO servers, but nice to use
						this->player->character->ShowBoard(boardid);
						return true;
					}

					if (post->time + static_cast<int>(this->server->world->config["BoardRecentPostTime"]) > Timer::GetTime())
					{
						++recent_post_count;

						if (recent_post_count >= static_cast<int>(this->server->world->config["BoardMaxUserRecentPosts"]))
						{
							// Not in the official EO servers, but nice to use
							this->player->character->ShowBoard(boardid);
							return true;
						}
					}
				}
			}

			Board_Post *newpost = new Board_Post;
			newpost->id = ++this->server->world->boards[boardid]->last_id;
			newpost->author = this->player->character->name;
			newpost->author_admin = this->player->character->admin;
			newpost->subject = subject;
			newpost->body = body;
			newpost->time = Timer::GetTime();

			this->server->world->boards[boardid]->posts.push_front(newpost);

			if (this->server->world->boards[boardid]->posts.size() > static_cast<std::size_t>(static_cast<int>(this->server->world->config["BoardMaxPosts"])))
			{
				this->server->world->boards[boardid]->posts.pop_back();
			}

			// Not in the official EO servers, but nice to use
			this->player->character->ShowBoard(boardid);
		}
		break;

		case PACKET_TAKE: // Reading a post on a town board
		{
			if (this->state < EOClient::Playing) return false;

			short boardid = reader.GetShort();
			short postid = reader.GetShort();

			if (static_cast<std::size_t>(boardid) > this->server->world->boards.size())
			{
				return true;
			}

			UTIL_PTR_LIST_FOREACH(this->server->world->boards[boardid]->posts, Board_Post, post)
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
		break;

		case PACKET_OPEN: // Opening town board
		{
			if (this->state < EOClient::Playing) return false;

			short boardid = reader.GetShort();

			this->player->character->ShowBoard(boardid);
		}
		break;

		default:
			return false;
	}

	return true;
}
