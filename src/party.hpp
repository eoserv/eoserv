
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef PARTY_HPP_INCLUDED
#define PARTY_HPP_INCLUDED

#include "stdafx.h"

/**
 * A temporary group of Characters
 */
class Party
{
	public:
		World *world;

		Character *leader;
		std::vector<Character *> members;

		int temp_expsum;

		Party(World *world, Character *leader, Character *other);

		void Msg(Character *from, std::string message);
		void Join(Character *);
		void Leave(Character *);
		void RefreshMembers(Character *);
		void UpdateHP(Character *);
		void ShareEXP(int exp, int sharemode, Map *map);

		~Party();
};

#endif // PARTY_HPP_INCLUDED
