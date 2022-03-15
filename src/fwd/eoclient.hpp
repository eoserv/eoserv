/* fwd/eoclient.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef FWD_EOCLIENT_HPP_INCLUDED
#define FWD_EOCLIENT_HPP_INCLUDED

class EOClient;
class ActionQueue;

struct ActionQueue_Action;

enum InitReply : unsigned char
{
	INIT_OUT_OF_DATE = 1,
	INIT_OK = 2,
	INIT_BANNED = 3,
	INIT_FILE_MAP = 4,
	INIT_FILE_EIF = 5,
	INIT_FILE_ENF = 6,
	INIT_FILE_ESF = 7,
	INIT_PLAYERS = 8,
	INIT_MAP_MUTATION = 9,
	INIT_FRIEND_LIST_PLAYERS = 10,
	INIT_FILE_ECF = 11
};

enum InitBanType : unsigned char
{
	INIT_BAN_TEMP = 0,
	INIT_BAN_PERM = 2
};

#endif // FWD_EOCLIENT_HPP_INCLUDED
