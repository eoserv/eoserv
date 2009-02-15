
#include "eoserv.hpp"

#include <list>
#include <map>
#include <ctime>

#include "packet.hpp"

void World::Msg(Player *from, std::string message)
{
	PacketBuilder builder;

	builder.SetID(PACKET_TALK, PACKET_MSG);
	builder.AddBreakString(from->name);
	builder.AddBreakString(message);

	for (std::list<Player *>::iterator it = this->players.begin(); it != this->players.end(); ++it)
	{
		if (*it == from)
		{
			continue;
		}

		(*it)->client->SendBuilder(builder);
	}
}
