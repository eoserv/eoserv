
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef PARTY_HPP_INCLUDED
#define PARTY_HPP_INCLUDED

#include <vector>
#include <string>

class Party;

#include "character.hpp"

/**
 * A temporary group of Characters
 */
class Party
{
	public:
		Party(Character *host, Character *other);

		Character *host;
		std::vector<Character *> members;

		void Msg(Character *from, std::string message);
		void Join(Character *);
		void Part(Character *);
};

#endif // PARTY_HPP_INCLUDED
