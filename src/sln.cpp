
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "sln.hpp"

#include <pthread.h>

#include "nanohttp.hpp"
#include "timer.hpp"
#include "eoserver.hpp"

namespace SLN
{

HTTP *http;
TimeEvent *tick_request_timer;

void request(void *server_void)
{
	pthread_t *thread = new pthread_t;
	pthread_create(thread, 0, real_request, server_void);
	pthread_detach(*thread);
}

void *real_request(void *server_void)
{
	EOServer *server = static_cast<EOServer *>(server_void);

	if (tick_request_timer != 0)
	{
		return 0;
	}

	std::string url = server->world->config["SLNURL"];
	url += "check?software=EOSERV";
	url += std::string("&retry=") + static_cast<std::string>(server->world->config["SLNPeriod"]);

	if (static_cast<std::string>(server->world->config["SLNHost"]).length() > 0)
	{
		url += std::string("&host=") + HTTP::URLEncode(server->world->config["SLNHost"]);
	}

	url += std::string("&port=") + HTTP::URLEncode(server->world->config["Port"]);
	url += std::string("&name=") + HTTP::URLEncode(server->world->config["ServerName"]);

	if (static_cast<std::string>(server->world->config["SLNSite"]).length() > 0)
	{
		url += std::string("&url=") + HTTP::URLEncode(server->world->config["SLNSite"]);
	}

	if (static_cast<std::string>(server->world->config["SLNZone"]).length() > 0)
	{
		url += std::string("&zone=") + HTTP::URLEncode(server->world->config["SLNZone"]);
	}

	try
	{
		if (static_cast<int>(server->world->config["SLNBind"]) == 0)
		{
			http = HTTP::RequestURL(url);
		}
		else if (static_cast<int>(server->world->config["SLNBind"]) == 1)
		{
			http = HTTP::RequestURL(url, IPAddress(static_cast<std::string>(server->world->config["Host"])));
		}
		else
		{
			http = HTTP::RequestURL(url, IPAddress(static_cast<std::string>(server->world->config["SLNBind"])));
		}
	}
	catch (Socket_Exception &e)
	{
		std::fputs(e.error(), stderr);
		return 0;
	}
	catch (...)
	{
		std::fputs("There was a problem trying to make the HTTP request...", stderr);
		return 0;
	}

	tick_request_timer = new TimeEvent(tick_request, server_void, 0.01, Timer::FOREVER, false);
	server->world->timer.Register(tick_request_timer);

	return 0;
}

void tick_request(void *server_void)
{
	EOServer *server = static_cast<EOServer *>(server_void);

	if (http == 0)
	{
		return;
	}

	http->Tick(0);

	if (http->Done())
	{
		std::vector<std::string> lines = util::explode("\r\n", http->Response());
		UTIL_VECTOR_FOREACH_ALL(lines, std::string, line)
		{
			if (line.length() == 0)
			{
				continue;
			}

			std::vector<std::string> parts = util::explode('\t', line);

			int code = util::to_int(parts[0]);
			int maincode = code / 100;

			std::string errmsg = std::string("(") + parts[0] + ") ";
			bool resolved = false;

			switch (maincode)
			{
				case 1: // Informational
					break;

				case 2: // Success
					break;

				case 3: // Warning
					errmsg += "SLN Update Warning: ";
					switch (code)
					{
						case 300:
							errmsg += parts[4];

							if (parts[2] == "retry")
							{
								server->world->config["SLNPeriod"] = util::to_int(parts[3]);
								resolved = true;
							}
							else if (parts[2] == "name")
							{
								server->world->config["ServerName"] = parts[3];
								resolved = true;
							}
							else if (parts[2] == "url")
							{
								server->world->config["SLNSite"] = parts[3];
								resolved = true;
							}
							break;

						case 301:
							errmsg += parts[2];
							break;

						case 302:
							errmsg += parts[2];
							break;

						default:
							errmsg += "Unknown error code";
							break;
					}

					fputs(errmsg.c_str(), stderr);
					fputs("\n", stderr);
					server->world->AdminMsg(0, errmsg, ADMIN_HGM);
					if (resolved)
					{
						server->world->AdminMsg(0, "EOSERV has automatically resolved this message and the next check-in should succeed.", ADMIN_HGM);
					}
					break;

				case 4: // Client Error
					errmsg += "SLN Update Client Error: ";
					switch (code)
					{
						case 400:
							errmsg += parts[3];
							break;

						case 401:
							errmsg += parts[3];

							if (parts[2] == "url")
							{
								server->world->config["SLNSite"] = "";
								resolved = true;
							}
							break;

						case 402:
							errmsg += parts[2];
							break;

						case 403:
							errmsg += parts[2];
							break;

						case 404:
							errmsg += parts[2];
							break;

						default:
							errmsg += "Unknown error code";
							break;
					}

					fputs(errmsg.c_str(), stderr);
					fputs("\n", stderr);
					server->world->AdminMsg(0, errmsg, ADMIN_HGM);
					if (resolved)
					{
						server->world->AdminMsg(0, "EOSERV has automatically resolved this message and the next check-in should succeed.", ADMIN_HGM);
					}
					break;

				case 5: // Server Error
					errmsg += "SLN Update Server Error: ";

					switch (code)
					{
						case 500:
							errmsg += parts[2];
							break;

						default:
							errmsg += "Unknown error code";
							break;

					}

					fputs(errmsg.c_str(), stderr);
					fputs("\n", stderr);
					server->world->AdminMsg(0, errmsg, ADMIN_HGM);
					break;
			}
		}

		delete http;
		http = 0;

		if (tick_request_timer != 0)
		{
			server->world->timer.Unregister(tick_request_timer);
		}

		tick_request_timer = 0;

		if (static_cast<int>(server->world->config["SLN"]))
		{
			server->world->timer.Register(new TimeEvent(request, server_void, int(server->world->config["SLNPeriod"]), 1, true));
		}
	}
}

}
