
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef EOSERVER_HPP_INCLUDED
#define EOSERVER_HPP_INCLUDED

#include "stdafx.h"

#include "socket.hpp"

void server_ping_all(void *server_void);
void server_pump_queue(void *server_void);

/**
 * A server which accepts connections and creates EOClient instances from them
 */
class EOServer : public Server
{
	private:
		void Initialize(util::array<std::string, 5> dbinfo, const Config &eoserv_config, const Config &admin_config);
		SLN *sln;

	protected:
		Client *ClientFactory(SOCKET sock, sockaddr_in sin);

	public:
		World *world;

		EOServer(IPAddress addr, unsigned short port, util::array<std::string, 5> dbinfo, const Config &eoserv_config, const Config &admin_config) : Server(addr, port)
		{
			this->Initialize(dbinfo, eoserv_config, admin_config);
		}

		~EOServer();

	SCRIPT_REGISTER_REF(EOServer)

	SCRIPT_REGISTER_END()
};

#endif // EOSERVER_HPP_INCLUDED
