
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef EOSERVER_HPP_INCLUDED
#define EOSERVER_HPP_INCLUDED

#include <string>
#include <vector>

class EOServer;

struct EOServer_Ban;

#include "util.hpp"
#include "socket.hpp"
#include "eoclient.hpp"
#include "config.hpp"
#include "world.hpp"

void server_ping_all(void *server_void);
void server_pump_queue(void *server_void);

/**
 * Information about a temporary in-memory ban
 */
struct EOServer_Ban
{
	std::string username;
	IPAddress address;
	std::string hdid;
	double expires;
};

/**
 * A server which accepts connections and creates EOClient instances from them
 */
class EOServer : public Server<EOClient>
{
	private:
		void Initialize(util::array<std::string, 5> dbinfo, const Config &eoserv_config, const Config &admin_config);
		std::vector<EOServer_Ban> bans;

	public:
		World *world;

		EOServer(IPAddress addr, unsigned short port, util::array<std::string, 5> dbinfo, const Config &eoserv_config, const Config &admin_config) : Server<EOClient>(addr, port)
		{
			this->Initialize(dbinfo, eoserv_config, admin_config);
		}

		void AddBan(std::string username, IPAddress address, std::string hdid, double duration);

		bool UsernameBanned(std::string username);
		bool AddressBanned(IPAddress address);
		bool HDIDBanned(std::string hdid);

		~EOServer();
};

#endif // EOSERVER_HPP_INCLUDED
