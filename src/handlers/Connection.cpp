
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "eoclient.hpp"

namespace Handlers
{

// Confirmation of initialization data
void Connection_Accept(EOClient *client, PacketReader &reader)
{
	(void)client;
	(void)reader;
}

// Ping reply
void Connection_Ping(EOClient *client, PacketReader &reader)
{
	(void)reader;

	client->needpong = false;
}

PACKET_HANDLER_REGISTER(PACKET_CONNECTION)
	Register(PACKET_ACCEPT, Connection_Accept, Menu);
	Register(PACKET_PING, Connection_Ping, Any | OutOfBand);
PACKET_HANDLER_REGISTER_END()

}
