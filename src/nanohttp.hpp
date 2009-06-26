
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef NANOHTTP_HPP_INCLUDED
#define NANOHTTP_HPP_INCLUDED

#include "socket.hpp"

/**
 * Super simple HTTP client
 */
class HTTP
{
	private:
		Client *client;
		std::string response;
		int status;
		bool done;

	public:
		HTTP(std::string host, unsigned short port, std::string path, IPAddress outgoing = 0U);

		static HTTP *RequestURL(std::string url, IPAddress outgoing = 0U);

		void Tick(int timeout);

		bool Done();
		int StatusCode();
		std::string Response();

		static std::string URLEncode(std::string);

		~HTTP();
};

#endif // NANOHTTP_HPP_INCLUDED
