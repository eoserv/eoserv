
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef GUILD_HPP_INCLUDED
#define GUILD_HPP_INCLUDED

#include "stdafx.h"

/**
 * Stores guild information and references to online members
 * Created by the World object when a member of the guild logs in, and destroyed when the last member logs out
 */
class Guild : public Shared
{
	public:
		std::string tag;
		std::string name;
		PtrVector<Character> members;
		util::array<std::string, 9> ranks;
		std::time_t created;

		void Msg(PtrVector<Character> from, std::string message);

	SCRIPT_REGISTER_REF_DF(Guild)

	SCRIPT_REGISTER_END()
};

#endif // GUILD_HPP_INCLUDED
