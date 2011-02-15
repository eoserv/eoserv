
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef EOSERVER_HPP_INCLUDED
#define EOSERVER_HPP_INCLUDED

#include "fwd/eoserver.hpp"

#include <array>
#include <string>

#include "socket.hpp"

#include "fwd/config.hpp"
#include "fwd/eoclient.hpp"
#include "fwd/sln.hpp"
#include "fwd/world.hpp"

void server_ping_all(void *server_void);
void server_pump_queue(void *server_void);

/**
 * A server which accepts connections and creates EOClient instances from them
 */
class EOServer : public Server
{
	private:
		void Initialize(std::array<std::string, 6> dbinfo, const Config &eoserv_config, const Config &admin_config);

	protected:
		virtual Client *ClientFactory(const Socket &);

	public:
		World *world;
		double start;
		SLN *sln;

		EOServer(IPAddress addr, unsigned short port, std::array<std::string, 6> dbinfo, const Config &eoserv_config, const Config &admin_config) : Server(addr, port)
		{
			this->Initialize(dbinfo, eoserv_config, admin_config);
		}

		~EOServer();
};

#endif // EOSERVER_HPP_INCLUDED
