/* handlers/Connection.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

#include "../eoclient.hpp"

#include "../console.hpp"

namespace Handlers
{

// Confirmation of initialization data
void Connection_Accept(EOClient *client, PacketReader &reader)
{
	unsigned emulti_d = reader.GetShort();
	unsigned emulti_e = reader.GetShort();
	unsigned client_id = reader.GetShort();

	auto multis = client->processor.GetEMulti();

	if (multis.first != emulti_e || multis.second != emulti_d || client->id != client_id)
	{
		client->server()->RecordClientRejection(client->GetRemoteAddr(), "bad connection data");
		client->Close(true);
		return;
	}

	client->server()->ClearClientRejections(client->GetRemoteAddr());

	client->MarkAccepted();
	Console::Out("Accepted connection from %s / %08X (%i/%i connections)", std::string(client->GetRemoteAddr()).c_str(), client->hdid, client->server()->Connections(), client->server()->MaxConnections());
}

// Ping reply
void Connection_Ping(EOClient *client, PacketReader &reader)
{
	(void)reader;

	if (client->needpong)
	{
		client->needpong = false;
	}
}

PACKET_HANDLER_REGISTER(PACKET_CONNECTION)
	Register(PACKET_ACCEPT, Connection_Accept, Menu);
	Register(PACKET_PING, Connection_Ping, Any | OutOfBand);
PACKET_HANDLER_REGISTER_END(PACKET_CONNECTION)

}
