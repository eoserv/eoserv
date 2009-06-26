
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef GUILD_HPP_INCLUDED
#define GUILD_HPP_INCLUDED

#include <vector>
#include <string>
#include <ctime>

class Guild;

#include "character.hpp"

#include "util.hpp"

/**
 * Stores guild information and references to online members
 * Created by the World object when a member of the guild logs in, and destroyed when the last member logs out
 */
class Guild
{
	public:
		std::string tag;
		std::string name;
		std::vector<Character *> members;
		util::array<std::string, 9> ranks;
		std::time_t created;

		void Msg(Character *from, std::string message);
};

#endif // GUILD_HPP_INCLUDED
