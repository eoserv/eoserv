/* fwd/player.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef FWD_PLAYER_HPP_INCLUDED
#define FWD_PLAYER_HPP_INCLUDED

class Player;

enum CharacterReply : short
{
	CHARACTER_EXISTS = 1,
	CHARACTER_FULL = 2,
	CHARACTER_NOT_APPROVED = 4,
	CHARACTER_OK = 5,
	CHARACTER_DELETED = 6,
};

#endif // FWD_PLAYER_HPP_INCLUDED
