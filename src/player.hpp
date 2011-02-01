
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef PLAYER_HPP_INCLUDED
#define PLAYER_HPP_INCLUDED

#include "fwd/player.hpp"

#include <string>

#include "container/ptr_vector.hpp"
#include "shared.hpp"

#include "fwd/character.hpp"
#include "fwd/eoclient.hpp"
#include "fwd/world.hpp"

/**
 * Object representing a player, but not a character
 */
class Player : public Shared
{
	public:
		int login_time;
		bool online;
		unsigned int id;
		std::string username;
		std::string password;

		Player(std::string username, World *);

		PtrVector<Character> characters;
		Character *character;

		static bool ValidName(std::string username);
		bool AddCharacter(std::string name, Gender gender, int hairstyle, int haircolor, Skin race);
		void ChangePass(std::string password);

		void Logout();

		World *world;
		EOClient *client;

		~Player();

	static Player *ScriptFactory(std::string username, World *world) { return new Player(username, world); }

	SCRIPT_REGISTER_REF(Player)
		SCRIPT_REGISTER_FACTORY("Player @f(string username, World @world)", ScriptFactory);

		SCRIPT_REGISTER_VARIABLE("int", login_time);
		SCRIPT_REGISTER_VARIABLE("bool", online);
		SCRIPT_REGISTER_VARIABLE("uint", id);
		SCRIPT_REGISTER_VARIABLE("string", username);
		SCRIPT_REGISTER_VARIABLE("string", password);
		SCRIPT_REGISTER_VARIABLE("PtrVector<Character>", characters);
		SCRIPT_REGISTER_VARIABLE("Character @", character);
		SCRIPT_REGISTER_VARIABLE("World @", world);
		//SCRIPT_REGISTER_VARIABLE("EOClient @", client);
		SCRIPT_REGISTER_FUNCTION("bool AddCharacter(string name, Gender gender, int hairstyle, int haircolor, Skin race)", AddCharacter);
		SCRIPT_REGISTER_FUNCTION("void ChangePass(string password)", ChangePass);
		SCRIPT_REGISTER_FUNCTION("void Logout()", Logout);
	SCRIPT_REGISTER_END()
};


#endif // PLAYER_HPP_INCLUDED
