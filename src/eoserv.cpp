
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

	printf("Global message: <%s> %s (%i len)\n", from->name.c_str(), message.c_str(), message.length());

	for (std::list<Player *>::iterator it = this->players.begin(); it != this->players.end(); ++it)
	{
		if (*it == from)
		{
			puts("  Skipping send to self");
			continue;
		}

		printf("  Sending to %s...\n", (*it)->name.c_str());
		(*it)->client->SendBuilder(builder);
	}
}
