
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef EOCONST_HPP_INCLUDED
#define EOCONST_HPP_INCLUDED

#include "util.hpp"

enum AdminLevel UTIL_EXTEND_ENUM(unsigned char)
{
	ADMIN_PLAYER = 0,
	ADMIN_GUIDE = 1,
	ADMIN_GUARDIAN = 2,
	ADMIN_GM = 3,
	ADMIN_HGM = 4
};

enum Direction UTIL_EXTEND_ENUM(unsigned char)
{
	DIRECTION_DOWN = 0,
	DIRECTION_LEFT = 1,
	DIRECTION_UP = 2,
	DIRECTION_RIGHT = 3
};

enum SitState UTIL_EXTEND_ENUM(unsigned char)
{
	SIT_SITTING = 1,
	SIT_STANDING = 2
};

enum Emote UTIL_EXTEND_ENUM(unsigned char)
{
	EMOTE_HAPPY = 1,
	EMOTE_DEPRESSED = 2,
	EMOTE_SAD = 3,
	EMOTE_ANGRY = 4,
	EMOTE_CONFUSED = 5,
	EMOTE_SURPRISED = 6,
	EMOTE_HEARTS = 7,
	EMOTE_MOON = 8,
	EMOTE_SUICIDAL = 9,
	EMOTE_EMBARASSED = 10,
	EMOTE_DRUNK = 11,
	EMOTE_TRADE = 12,
	EMOTE_LEVELUP = 13,
	EMOTE_PLAYFUL = 14,
};

enum QuestAction UTIL_EXTEND_ENUM(unsigned char)
{
	QUEST_PROGRESS = 1,
	QUEST_HISTORY = 2
};

enum FileType UTIL_EXTEND_ENUM(unsigned char)
{
	FILE_MAP = 1,
	FILE_ITEM = 2,
	FILE_NPC = 3,
	FILE_SPELL = 4,
	FILE_CLASS = 5
};

enum Gender UTIL_EXTEND_ENUM(unsigned char)
{
	GENDER_FEMALE = 0,
	GENDER_MALE = 1
};

enum Skin UTIL_EXTEND_ENUM(unsigned char)
{
	SKIN_WHITE = 0,
	SKIN_YELLOW = 1,
	SKIN_TAN = 2,
	SKIN_ORC = 3,
	SKIN_PANDA = 4,
	SKIN_SKELETON = 5,
	SKIN_FISH = 6
};

enum PaperdollIcon UTIL_EXTEND_ENUM(unsigned char)
{
	ICON_NORMAL = 0,
	ICON_GM = 4,
	ICON_HGM = 5,
	ICON_PARTY = 6,
	ICON_GM_PARTY = 9,
	ICON_HGM_PARTY = 10
};

enum PartyRequestType UTIL_EXTEND_ENUM(unsigned char)
{
	PARTY_REQUEST_JOIN = 0,
	PARTY_REQUEST_INVITE = 1,
};

enum InitReply UTIL_EXTEND_ENUM(unsigned char)
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
	INIT_NOISE = 10,
	INIT_FILE_ECF = 11
};

enum InitBanType UTIL_EXTEND_ENUM(unsigned char)
{
	INIT_BAN_TEMP = 0,
	INIT_BAN_PERM = 2
};

enum AccountReply UTIL_EXTEND_ENUM(unsigned short)
{
	ACCOUNT_EXISTS = 1,
	ACCOUNT_NOT_APPROVED = 2,
	ACCOUNT_CREATED = 3,
	ACCOUNT_CHANGE_FAILED = 5,
	ACCOUNT_CHANGED = 6,
	ACCOUNT_CONTINUE = 1000 // TODO: Check this for the real value
};

enum CharacterReply UTIL_EXTEND_ENUM(unsigned char)
{
	CHARACTER_EXISTS = 1,
	CHARACTER_FULL = 2,
	CHARACTER_NOT_APPROVED = 4,
	CHARACTER_OK = 5,
	CHARACTER_DELETED = 6,
};

enum LoginReply UTIL_EXTEND_ENUM(unsigned char)
{
	LOGIN_WRONG_USER = 1,
	LOGIN_WRONG_USERPASS = 2,
	LOGIN_OK = 3,
	LOGIN_LOGGEDIN = 5
};

enum WarpReply UTIL_EXTEND_ENUM(unsigned char)
{
	WARP_LOCAL = 1,
	WARP_SWITCH = 2,
};

enum TalkReply UTIL_EXTEND_ENUM(unsigned char)
{
	TALK_NOTFOUND = 1
};

enum SitAction UTIL_EXTEND_ENUM(unsigned char)
{
	SIT_STAND = 0,
	SIT_CHAIR = 1,
	SIT_FLOOR = 2
};

enum WarpAnimation UTIL_EXTEND_ENUM(unsigned char)
{
	WARP_ANIMATION_NONE = 0,
	WARP_ANIMATION_SCROLL = 1,
	WARP_ANIMATION_ADMIN = 2,
	WARP_ANIMATION_INVALID = 255,
};

enum MapEffect UTIL_EXTEND_ENUM(unsigned char)
{
	MAP_EFFECT_QUAKE = 1
};


#endif // EOCONST_HPP_INCLUDED
