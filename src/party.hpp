
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef PARTY_HPP_INCLUDED
#define PARTY_HPP_INCLUDED

#include "container/ptr_vector.hpp"
#include "script.hpp"
#include "shared.hpp"

#include "fwd/character.hpp"
#include "fwd/map.hpp"
#include "fwd/world.hpp"

/**
 * A temporary group of Characters
 */
class Party : public Shared
{
	public:
		World *world;

		Character *leader;
		PtrVector<Character> members;

		int temp_expsum;

		Party(World *world, Character *leader, Character *other);

		void Msg(Character *from, std::string message, bool echo = true);
		void Join(Character *);
		void Leave(Character *);
		void RefreshMembers(Character *);
		void UpdateHP(Character *);
		void ShareEXP(int exp, int sharemode, Map *map);

		~Party();

	static Party *ScriptFactory(World *world, Character *leader, Character *other) { return new Party(world, leader, other); }

	SCRIPT_REGISTER_REF(Party)
		SCRIPT_REGISTER_FACTORY("Party @f(World @, Character @, Character @)", ScriptFactory);

		SCRIPT_REGISTER_VARIABLE("World @", world);
		SCRIPT_REGISTER_VARIABLE("Character @", leader);
		SCRIPT_REGISTER_VARIABLE("PtrVector<Character>", members);
		SCRIPT_REGISTER_VARIABLE("int", temp_expsum);
		SCRIPT_REGISTER_FUNCTION("void Msg(Character @from, string message, bool echo)", Msg);
		SCRIPT_REGISTER_FUNCTION("void Join(Character @)", Join);
		SCRIPT_REGISTER_FUNCTION("void Leave(Character @)", Leave);
		SCRIPT_REGISTER_FUNCTION("void RefreshMembers(Character @)", RefreshMembers);
		SCRIPT_REGISTER_FUNCTION("void UpdateHP(Character @)", UpdateHP);
		SCRIPT_REGISTER_FUNCTION("void ShareEXP(int exp, int sharemode, Map @)", ShareEXP);
	SCRIPT_REGISTER_END()
};

#endif // PARTY_HPP_INCLUDED
