
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.h"

#include "character.hpp"
#include "map.hpp"
#include "party.hpp"
#include "player.hpp"

CLIENT_F_FUNC(Party)
{
	PacketBuilder reply;

	switch (action)
	{
		case PACKET_REQUEST: // Invite/Join request
		{
			if (this->state < EOClient::Playing) return false;

			PartyRequestType type = static_cast<PartyRequestType>(reader.GetChar());
			unsigned short invitee = reader.GetShort();

			if (type != PARTY_REQUEST_JOIN && type != PARTY_REQUEST_INVITE)
			{
				return false;
			}

			UTIL_PTR_LIST_FOREACH(this->player->character->map->characters, Character, character)
			{
				if (character->player->id == invitee && this->player->character->InRange(*character))
				{
					PacketBuilder builder(PACKET_PARTY, PACKET_REQUEST);
					builder.AddChar(type);
					builder.AddShort(this->player->id);
					builder.AddString(this->player->character->name);

					character->player->client->SendBuilder(builder);

					this->player->character->party_trust_send = *character;
					character->party_trust_recv = this->player->character;
					this->player->character->party_send_type = type;
					break;
				}
			}
		}
		break;

		case PACKET_ACCEPT: // Accept invite/join request
		{
			if (this->state < EOClient::PlayingModal) return false;

			/*PartyRequestType type = */static_cast<PartyRequestType>(reader.GetChar());
			unsigned short inviter_id = reader.GetShort();
			Character *inviter = 0;

			UTIL_PTR_LIST_FOREACH(this->player->character->map->characters, Character, character)
			{
				if (character->player->id == inviter_id && this->player->character->InRange(*character))
				{
					if (character->party_trust_send == this->player->character)
					{
						inviter = *character;
					}
					break;
				}
			}

			if (!inviter)
			{
				return true;
			}

			if (this->player->character->party_trust_recv == inviter)
			{
				if (inviter->party_send_type == PARTY_REQUEST_JOIN)
				{
					if (inviter->party)
					{
						inviter->party->Leave(inviter);
					}

					if (!this->player->character->party)
					{
						new Party(this->server()->world, this->player->character, inviter);
					}
					else
					{
						this->player->character->party->Join(inviter);
					}
				}
				else if (inviter->party_send_type == PARTY_REQUEST_INVITE)
				{
					if (this->player->character->party)
					{
						this->player->character->party->Leave(this->player->character);
					}

					if (!inviter->party)
					{
						new Party(this->server()->world, inviter, this->player->character);
					}
					else
					{
						inviter->party->Join(this->player->character);
					}
				}

				this->player->character->party_trust_recv = 0;
				inviter->party_trust_send = 0;
			}
		}
		break;

		case PACKET_REMOVE: // Remove a player from a party
		{
			if (this->state < EOClient::PlayingModal) return false;
			if (!this->player->character->party) return false;

			unsigned short id = reader.GetShort();

			if (id == this->player->id || this->player->character == this->player->character->party->leader)
			{
				this->player->character->party->Leave(this->server()->world->GetCharacterPID(id));
			}
			else
			{
				return false;
			}
		}
		break;

		case PACKET_TAKE: // Requested list of party members
		{
			if (this->state < EOClient::PlayingModal) return false;
			if (!this->player->character->party) return false;

			// This doesn't really need to exist
		}
		break;

		default:
			return false;
	}

	return true;
}
