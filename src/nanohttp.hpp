
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef NANOHTTP_HPP_INCLUDED
#define NANOHTTP_HPP_INCLUDED

#include "stdafx.h"

#include "socket.hpp"

/**
 * Super simple HTTP client
 */
class HTTP : public Shared
{
	private:
		Client *client;
		std::string response;
		int status;
		bool done;

	public:
		HTTP(std::string host, unsigned short port, std::string path, const IPAddress &outgoing = 0U);

		static HTTP *RequestURL(std::string url, const IPAddress &outgoing = 0U);

		void Tick(double timeout);

		bool Done();
		int StatusCode();
		std::string Response();

		static std::string URLEncode(std::string);

		~HTTP();

	static HTTP *ScriptFactory(std::string host, unsigned short port, std::string path, const IPAddress &outgoing) { return new HTTP(host, port, path, outgoing); }

	SCRIPT_REGISTER_REF(HTTP)
		SCRIPT_REGISTER_FACTORY("HTTP @f(string host, uint16 port, string path, const IPAddress &outgoing)", ScriptFactory);

		SCRIPT_REGISTER_FUNCTION("void Tick(double timeout)", Tick);
		SCRIPT_REGISTER_FUNCTION("bool Done()", Done);
		SCRIPT_REGISTER_FUNCTION("int StatusCode()", StatusCode);
		SCRIPT_REGISTER_FUNCTION("string Response()", Response);

		SCRIPT_REGISTER_GLOBAL_FUNCTION("HTTP @HTTP_RequestURL(string url, const IPAddress &outgoing)", RequestURL);
		SCRIPT_REGISTER_GLOBAL_FUNCTION("string HTTP_URLEncode(string)", URLEncode);
	SCRIPT_REGISTER_END()
};

#endif // NANOHTTP_HPP_INCLUDED
