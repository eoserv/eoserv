/* handlers/Character.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../config.hpp"
#include "../eodata.hpp"
#include "../packet.hpp"
#include "../player.hpp"
#include "../world.hpp"

#include "../console.hpp"
#include "../util.hpp"

#include <algorithm>
#include <cstddef>
#include <string>

namespace Handlers
{

void Character_Request(Player *player, PacketReader &reader)
{
	(void)reader;

	PacketBuilder reply(PACKET_CHARACTER, PACKET_REPLY, 4);
	reply.AddShort(1000); // CreateID?
	reply.AddString("OK");

	player->Send(reply);
}

void Character_Create(Player *player, PacketReader &reader)
{
	reader.GetShort(); // CreateID?

	Gender gender = static_cast<Gender>(reader.GetShort());
	int hairstyle = reader.GetShort();
	int haircolor = reader.GetShort();
	Skin race = static_cast<Skin>(reader.GetShort());
	reader.GetByte();
	std::string name = reader.GetBreakString();
	name = util::lowercase(name);

	if ((gender != GENDER_MALE && gender != GENDER_FEMALE)
	 || hairstyle < static_cast<int>(player->world->config["CreateMinHairStyle"])
	 || hairstyle > static_cast<int>(player->world->config["CreateMaxHairStyle"])
	 || haircolor < static_cast<int>(player->world->config["CreateMinHairColor"])
	 || haircolor > static_cast<int>(player->world->config["CreateMaxHairColor"])
	 || race < static_cast<int>(player->world->config["CreateMinSkin"])
	 || race > static_cast<int>(player->world->config["CreateMaxSkin"]))
	return;

	PacketBuilder reply(PACKET_CHARACTER, PACKET_REPLY, 2);

	if (player->characters.size() >= static_cast<std::size_t>(static_cast<int>(player->world->config["MaxCharacters"])))
	{
		reply.AddShort(CHARACTER_FULL); // Reply code
	}
	else if (!Character::ValidName(name))
	{
		reply.AddShort(CHARACTER_NOT_APPROVED); // Reply code
	}
	else if (player->world->CharacterExists(name))
	{
		reply.AddShort(CHARACTER_EXISTS); // Reply code
	}
	else
	{
		player->AddCharacter(name, gender, hairstyle, haircolor, race);
		reply.ReserveMore(5 + player->characters.size() * 34);
		Console::Out("New character: %s (%s)", name.c_str(), player->username.c_str());

		reply.AddShort(CHARACTER_OK);
		reply.AddChar(player->characters.size());
		reply.AddByte(1); // ??
		reply.AddByte(255);
		UTIL_FOREACH(player->characters, character)
		{
			reply.AddBreakString(character->SourceName());
			reply.AddInt(character->id);
			reply.AddChar(character->level);
			reply.AddChar(character->gender);
			reply.AddChar(character->hairstyle);
			reply.AddChar(character->haircolor);
			reply.AddChar(character->race);
			reply.AddChar(character->admin);
			character->AddPaperdollData(reply, "BAHSW");

			reply.AddByte(255);
		}
	}

	player->Send(reply);
}

// Delete a character from an account
void Character_Remove(Player *player, PacketReader &reader)
{
	/*int deleteid = */reader.GetShort();
	unsigned int id = reader.GetInt();

	auto it = std::find_if(UTIL_RANGE(player->characters), [&](Character *c) -> bool
	{
		return (c->id == id);
	});

	if (it == player->characters.end())
		return;

	player->world->DeleteCharacter((*it)->real_name);
	Console::Out("Deleted character: %s (%s)", (*it)->real_name.c_str(), player->username.c_str());

	if ((*it)->admin > 0)
		player->world->DecAdminCount();

	player->characters.erase(it);

	PacketBuilder reply(PACKET_CHARACTER, PACKET_REPLY, 5 + player->characters.size() * 34);
	reply.AddShort(CHARACTER_DELETED); // Reply code
	reply.AddChar(player->characters.size());
	reply.AddByte(1); // ??
	reply.AddByte(255);
	UTIL_FOREACH(player->characters, character)
	{
		reply.AddBreakString(character->SourceName());
		reply.AddInt(character->id);
		reply.AddChar(character->level);
		reply.AddChar(character->gender);
		reply.AddChar(character->hairstyle);
		reply.AddChar(character->haircolor);
		reply.AddChar(character->race);
		reply.AddChar(character->admin);
		character->AddPaperdollData(reply, "BAHSW");

		reply.AddByte(255);
	}

	player->Send(reply);
}

// Request to delete a character from an account
void Character_Take(Player *player, PacketReader &reader)
{
	unsigned int id = reader.GetInt();

	auto it = std::find_if(UTIL_CRANGE(player->characters), [&](Character *c) -> bool
	{
		return (c->id == id);
	});

	if (it == player->characters.end())
		return;

	PacketBuilder reply(PACKET_CHARACTER, PACKET_PLAYER, 6);
	reply.AddShort(1000); // Delete req ID
	reply.AddInt(id);

	player->Send(reply);
}

PACKET_HANDLER_REGISTER(PACKET_CHARACTER)
	Register(PACKET_REQUEST, Character_Request, Character_Menu);
	Register(PACKET_CREATE, Character_Create, Character_Menu, 1.0);
	Register(PACKET_REMOVE, Character_Remove, Character_Menu, 1.0);
	Register(PACKET_TAKE, Character_Take, Character_Menu);
PACKET_HANDLER_REGISTER_END(PACKET_CHARACTER)

}
