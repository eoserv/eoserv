
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef PLAYER_HPP_INCLUDED
#define PLAYER_HPP_INCLUDED

#include "stdafx.h"

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

	SCRIPT_REGISTER_REF(Player)

	SCRIPT_REGISTER_END()
};


#endif // PLAYER_HPP_INCLUDED
