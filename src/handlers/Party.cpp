/* handlers/Party.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../character.hpp"
#include "../map.hpp"
#include "../packet.hpp"
#include "../party.hpp"
#include "../world.hpp"

namespace Handlers
{

// Invite/Join request
void Party_Request(Character *character, PacketReader &reader)
{
	PartyRequestType type = static_cast<PartyRequestType>(reader.GetChar());
	unsigned short invitee_id = reader.GetShort();

	if (type != PARTY_REQUEST_JOIN && type != PARTY_REQUEST_INVITE)
	{
		return;
	}

	Character *invitee = character->map->GetCharacterPID(invitee_id);

	if (!invitee || character == invitee || !character->InRange(invitee))
		return;

	PacketBuilder builder(PACKET_PARTY, PACKET_REQUEST, 3 + character->SourceName().length());
	builder.AddChar(type);
	builder.AddShort(character->PlayerID());
	builder.AddString(character->SourceName());

	invitee->Send(builder);

	character->party_trust_send = invitee;
	invitee->party_trust_recv = character;
	character->party_send_type = type;
}

// Accept invite/join request
void Party_Accept(Character *character, PacketReader &reader)
{
	PartyRequestType type = static_cast<PartyRequestType>(reader.GetChar());
	unsigned short inviter_id = reader.GetShort();

	(void)type;

	Character *inviter = character->map->GetCharacterPID(inviter_id);

	if (!inviter || !character->InRange(inviter)
	 || inviter->party_trust_send != character
	 || character->party_trust_recv != inviter)
	{
		return;
	}

	if (inviter->party_send_type == PARTY_REQUEST_JOIN)
	{
		if (inviter->party)
		{
			inviter->party->Leave(inviter);
		}

		if (!character->party)
		{
			new Party(character->world, character, inviter);
		}
		else
		{
			character->party->Join(inviter);
		}
	}
	else if (inviter->party_send_type == PARTY_REQUEST_INVITE)
	{
		if (character->party)
		{
			character->party->Leave(character);
		}

		if (!inviter->party)
		{
			new Party(character->world, inviter, character);
		}
		else
		{
			inviter->party->Join(character);
		}
	}

	character->party_trust_recv = 0;
	inviter->party_trust_send = 0;
}

// Remove a player from a party
void Packet_Remove(Character *character, PacketReader &reader)
{
	if (!character->party) return;

	unsigned short id = reader.GetShort();

	if (id == character->PlayerID() || character == character->party->leader)
	{
		Character *leaver = character->world->GetCharacterPID(id);

		if (!leaver)
			return;

		character->party->Leave(leaver);
	}
	else
	{
		return;
	}
}

// Requested list of party members
void Party_Take(Character *character, PacketReader &reader)
{
	(void)reader;

	if (!character->party) return;

	character->party->RefreshMembers(character);
}

PACKET_HANDLER_REGISTER(PACKET_PARTY)
	Register(PACKET_REQUEST, Party_Request, Playing, 0.5);
	Register(PACKET_ACCEPT, Party_Accept, Playing);
	Register(PACKET_REMOVE, Packet_Remove, Playing);
	Register(PACKET_TAKE, Party_Take, Playing);
PACKET_HANDLER_REGISTER_END(PACKET_PARTY)

}
