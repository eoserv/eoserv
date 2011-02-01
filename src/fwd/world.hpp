
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef FWD_WORLD_HPP_INCLUDED
#define FWD_WORLD_HPP_INCLUDED

class World;

struct Board;
struct Board_Post;
struct Home;

enum AccountReply
{
	ACCOUNT_EXISTS = 1,
	ACCOUNT_NOT_APPROVED = 2,
	ACCOUNT_CREATED = 3,
	ACCOUNT_CHANGE_FAILED = 5,
	ACCOUNT_CHANGED = 6,
	ACCOUNT_CONTINUE = 1000 // TODO: Check this for the real value
};

enum LoginReply
{
	LOGIN_WRONG_USER = 1,
	LOGIN_WRONG_USERPASS = 2,
	LOGIN_OK = 3,
	LOGIN_LOGGEDIN = 5,
	LOGIN_BUSY = 6
};

enum WarpAnimation
{
	WARP_ANIMATION_NONE = 0,
	WARP_ANIMATION_SCROLL = 1,
	WARP_ANIMATION_ADMIN = 2,
	WARP_ANIMATION_INVALID = 255,
};

enum WarpReply
{
	WARP_LOCAL = 1,
	WARP_SWITCH = 2,
};

#endif // FWD_WORLD_HPP_INCLUDED
