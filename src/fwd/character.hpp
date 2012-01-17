
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef FWD_CHARACTER_HPP_INCLUDED
#define FWD_CHARACTER_HPP_INCLUDED

class Character;

struct Character_Item;
struct Character_Spell;

enum AdminLevel
{
	ADMIN_PLAYER = 0,
	ADMIN_GUIDE = 1,
	ADMIN_GUARDIAN = 2,
	ADMIN_GM = 3,
	ADMIN_HGM = 4
};

enum Direction
{
	DIRECTION_DOWN = 0,
	DIRECTION_LEFT = 1,
	DIRECTION_UP = 2,
	DIRECTION_RIGHT = 3
};

enum Emote
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

enum QuestAction
{
	QUEST_PROGRESS = 1,
	QUEST_HISTORY = 2
};

enum Gender
{
	GENDER_FEMALE = 0,
	GENDER_MALE = 1
};

enum Skin
{
	SKIN_WHITE = 0,
	SKIN_YELLOW = 1,
	SKIN_TAN = 2,
	SKIN_ORC = 3,
	SKIN_SKELETON = 4,
	SKIN_PANDA = 5,
	SKIN_FISH = 6
};

enum PaperdollIcon
{
	ICON_NORMAL = 0,
	ICON_GM = 4,
	ICON_HGM = 5,
	ICON_PARTY = 6,
	ICON_GM_PARTY = 9,
	ICON_HGM_PARTY = 10
};

enum ClothesSlot
{
	SLOT_CLOTHES = 1,
	SLOT_HAIR = 2,
	SLOT_HAIRCOLOR = 3
};

enum TalkReply
{
	TALK_NOTFOUND = 1
};

enum SitState
{
	SIT_STAND = 0,
	SIT_CHAIR = 1,
	SIT_FLOOR = 2
};

enum SitAction
{
	SIT_ACT_SIT = 1,
	SIT_ACT_STAND = 2
};

enum TrainType
{
    TRAIN_STAT = 1,
    TRAIN_SKILL = 2
};

#endif // FWD_CHARACTER_HPP_INCLUDED
