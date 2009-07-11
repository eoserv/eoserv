
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "sln.hpp"

#include <string>
#include <vector>

#include <pthread.h>
#include <unistd.h>

#include "nanohttp.hpp"
#include "timer.hpp"
#include "util.hpp"
#include "console.hpp"

// TODO: Make this safe (race conditions)

SLN::SLN(EOServer *server)
{
	this->server = server;

	this->Request();
}

void SLN::Request()
{
	static pthread_t thread;

	if (pthread_create(&thread, 0, SLN::RequestThread, this) != 0)
	{
		throw std::exception();
	}

	pthread_detach(thread);
}

void *SLN::RequestThread(void *void_sln)
{
	HTTP *http;
	SLN *sln = static_cast<SLN *>(void_sln);

	std::string url = sln->server->world->config["SLNURL"];
	url += "check?software=EOSERV";
	url += std::string("&retry=") + static_cast<std::string>(sln->server->world->config["SLNPeriod"]);

	if (static_cast<std::string>(sln->server->world->config["SLNHost"]).length() > 0)
	{
		url += std::string("&host=") + HTTP::URLEncode(sln->server->world->config["SLNHost"]);
	}

	url += std::string("&port=") + HTTP::URLEncode(sln->server->world->config["Port"]);
	url += std::string("&name=") + HTTP::URLEncode(sln->server->world->config["ServerName"]);

	if (static_cast<std::string>(sln->server->world->config["SLNSite"]).length() > 0)
	{
		url += std::string("&url=") + HTTP::URLEncode(sln->server->world->config["SLNSite"]);
	}

	if (static_cast<std::string>(sln->server->world->config["SLNZone"]).length() > 0)
	{
		url += std::string("&zone=") + HTTP::URLEncode(sln->server->world->config["SLNZone"]);
	}

	try
	{
		if (static_cast<int>(sln->server->world->config["SLNBind"]) == 0)
		{
			http = HTTP::RequestURL(url);
		}
		else if (static_cast<int>(sln->server->world->config["SLNBind"]) == 1)
		{
			http = HTTP::RequestURL(url, IPAddress(static_cast<std::string>(sln->server->world->config["Host"])));
		}
		else
		{
			http = HTTP::RequestURL(url, IPAddress(static_cast<std::string>(sln->server->world->config["SLNBind"])));
		}
	}
	catch (Socket_Exception &e)
	{
		Console::Err(e.error());
		return 0;
	}
	catch (...)
	{
		Console::Err("There was a problem trying to make the HTTP request...");
		return 0;
	}

	while (!http->Done())
	{
		http->Tick(0.01);
	}

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
							sln->server->world->config["SLNPeriod"] = util::to_int(parts[3]);
							resolved = true;
						}
						else if (parts[2] == "name")
						{
							sln->server->world->config["ServerName"] = parts[3];
							resolved = true;
						}
						else if (parts[2] == "url")
						{
							sln->server->world->config["SLNSite"] = parts[3];
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

				Console::Wrn(errmsg);
				sln->server->world->AdminMsg(0, errmsg, ADMIN_HGM);
				if (resolved)
				{
					sln->server->world->AdminMsg(0, "EOSERV has automatically resolved this message and the next check-in should succeed.", ADMIN_HGM);
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
							sln->server->world->config["SLNSite"] = "";
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

				Console::Wrn(errmsg);
				sln->server->world->AdminMsg(0, errmsg, ADMIN_HGM);
				if (resolved)
				{
					sln->server->world->AdminMsg(0, "EOSERV has automatically resolved this message and the next check-in should succeed.", ADMIN_HGM);
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

				Console::Wrn(errmsg);
				sln->server->world->AdminMsg(0, errmsg, ADMIN_HGM);
				break;
		}
	}

	delete http;

	sln->server->world->timer.Register(new TimeEvent(SLN::TimedRequest, sln, static_cast<int>(sln->server->world->config["SLNPeriod"]), 1, true));

	return 0;
}

void SLN::TimedRequest(void *void_sln)
{
	SLN *sln = static_cast<SLN *>(void_sln);

	sln->Request();
}
