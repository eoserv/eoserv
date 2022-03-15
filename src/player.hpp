/* player.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef PLAYER_HPP_INCLUDED
#define PLAYER_HPP_INCLUDED

#include "fwd/player.hpp"

#include "fwd/character.hpp"
#include "fwd/eoclient.hpp"
#include "fwd/packet.hpp"
#include "fwd/world.hpp"

#include "util/secure_string.hpp"

#include <string>
#include <vector>

/**
 * Object representing a player, but not a character
 */
class Player
{
	public:
		int login_time;
		bool online;
		unsigned int id;
		std::string username;

		std::string dutylast;

		Player(std::string username, World *);

		std::vector<Character *> characters;
		Character *character;

		static bool ValidName(std::string username);
		bool AddCharacter(std::string name, Gender gender, int hairstyle, int haircolor, Skin race);
		void ChangePass(util::secure_string&& password);

		AdminLevel Admin() const;

		void Send(const PacketBuilder &);

		void Logout();

		World *world;
		EOClient *client;

		~Player();
};


#endif // PLAYER_HPP_INCLUDED
