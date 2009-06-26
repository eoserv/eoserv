
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

CLIENT_F_FUNC(Character)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Request to create a new character
		{
			if (this->state != EOClient::LoggedIn) return false;

			reply.SetID(PACKET_CHARACTER, PACKET_REPLY);
			reply.AddShort(1000); // CreateID?
			reply.AddString("OK");
			CLIENT_SEND(reply);
		}
		break;

		case PACKET_CREATE: // Create a character
		{
			if (this->state != EOClient::LoggedIn) return false;

			reader.GetShort(); // CreateID?

			Gender gender = static_cast<Gender>(reader.GetShort());
			int hairstyle = reader.GetShort();
			int haircolor = reader.GetShort();
			Skin race = static_cast<Skin>(reader.GetShort());
			reader.GetByte();
			std::string name = reader.GetBreakString();
			util::lowercase(name);

			if (gender != GENDER_MALE && gender != GENDER_FEMALE) return false;
			if (hairstyle < static_cast<int>(this->server->world->config["CreateMinHairStyle"]) || hairstyle > static_cast<int>(this->server->world->config["CreateMaxHairStyle"])) return false;
			if (haircolor < static_cast<int>(this->server->world->config["CreateMinHairColor"]) || haircolor > static_cast<int>(this->server->world->config["CreateMaxHairColor"])) return false;
			if (race < static_cast<int>(this->server->world->config["CreateMinSkin"]) || race > static_cast<int>(this->server->world->config["CreateMaxSkin"])) return false;

			reply.SetID(PACKET_CHARACTER, PACKET_REPLY);

			if (this->player->characters.size() >= static_cast<std::size_t>(static_cast<int>(this->server->world->config["MaxCharacters"])))
			{
				reply.AddShort(CHARACTER_FULL); // Reply code
			}
			else if (!Character::ValidName(name))
			{
				reply.AddShort(CHARACTER_NOT_APPROVED); // Reply code
			}
			else if (this->server->world->CharacterExists(name))
			{
				reply.AddShort(CHARACTER_EXISTS); // Reply code
			}
			else
			{
				this->player->AddCharacter(name, gender, hairstyle, haircolor, race);

				reply.AddShort(CHARACTER_OK);
				reply.AddChar(this->player->characters.size());
				reply.AddByte(1); // ??
				reply.AddByte(255);
				UTIL_VECTOR_FOREACH_ALL(this->player->characters, Character *, character)
				{
					reply.AddBreakString(character->name);
					reply.AddInt(character->id);
					reply.AddChar(character->level);
					reply.AddChar(character->gender);
					reply.AddChar(character->hairstyle);
					reply.AddChar(character->haircolor);
					reply.AddChar(character->race);
					reply.AddChar(character->admin);
					reply.AddShort(this->server->world->eif->Get(character->paperdoll[Character::Boots])->dollgraphic);
					reply.AddShort(this->server->world->eif->Get(character->paperdoll[Character::Armor])->dollgraphic);
					reply.AddShort(this->server->world->eif->Get(character->paperdoll[Character::Hat])->dollgraphic);
					reply.AddShort(this->server->world->eif->Get(character->paperdoll[Character::Shield])->dollgraphic);
					reply.AddShort(this->server->world->eif->Get(character->paperdoll[Character::Weapon])->dollgraphic);
					reply.AddByte(255);
				}
			}

			CLIENT_SEND(reply);

		}
		break;

		case PACKET_REMOVE: // Delete a character from an account
		{
			if (this->state != EOClient::LoggedIn) return false;

			/*int deleteid = */reader.GetShort();
			unsigned int charid = reader.GetInt();

			bool yourchar = false;
			std::vector<Character *>::iterator char_it;

			UTIL_VECTOR_IFOREACH(this->player->characters.begin(), this->player->characters.end(), Character *, character)
			{
				if ((*character)->id == charid)
				{
					this->server->world->DeleteCharacter((*character)->name);
					char_it = character;
					yourchar = true;
					break;
				}
			}

			if (!yourchar)
			{
				return false;
			}

			this->player->characters.erase(char_it);

			reply.SetID(PACKET_CHARACTER, PACKET_REPLY);
			reply.AddShort(CHARACTER_DELETED); // Reply code
			reply.AddChar(this->player->characters.size());
			reply.AddByte(1); // ??
			reply.AddByte(255);
			UTIL_VECTOR_FOREACH_ALL(this->player->characters, Character *, character)
			{
				reply.AddBreakString(character->name);
				reply.AddInt(character->id);
				reply.AddChar(character->level);
				reply.AddChar(character->gender);
				reply.AddChar(character->hairstyle);
				reply.AddChar(character->haircolor);
				reply.AddChar(character->race);
				reply.AddChar(character->admin);
				reply.AddShort(this->server->world->eif->Get(character->paperdoll[Character::Boots])->dollgraphic);
				reply.AddShort(this->server->world->eif->Get(character->paperdoll[Character::Armor])->dollgraphic);
				reply.AddShort(this->server->world->eif->Get(character->paperdoll[Character::Hat])->dollgraphic);
				reply.AddShort(this->server->world->eif->Get(character->paperdoll[Character::Shield])->dollgraphic);
				reply.AddShort(this->server->world->eif->Get(character->paperdoll[Character::Weapon])->dollgraphic);
				reply.AddByte(255);
			}
			CLIENT_SEND(reply);
		}
		break;

		case PACKET_TAKE: // Request to delete a character from an account
		{
			if (this->state != EOClient::LoggedIn) return false;

			unsigned int charid = reader.GetInt();

			bool yourchar = false;

			UTIL_VECTOR_FOREACH_ALL(this->player->characters, Character *, character)
			{
				if (character->id == charid)
				{
					yourchar = true;
					break;
				}
			}

			if (!yourchar)
			{
				return true;
			}

			reply.SetID(PACKET_CHARACTER, PACKET_PLAYER);
			reply.AddShort(1000); // Delete req ID
			reply.AddInt(charid);
			CLIENT_SEND(reply);
		}
		break;

		default:
			return false;
	}

	return true;
}
