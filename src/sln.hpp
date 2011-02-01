
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef SLN_HPP_INCLUDED
#define SLN_HPP_INCLUDED

#include "fwd/sln.hpp"

#include "script.hpp"
#include "shared.hpp"

#include "fwd/eoserver.hpp"

/**
 * Manages checking in with the SLN server regularly
 */
class SLN : public Shared
{
	private:
		EOServer *server;

		void RequestTick();
		static void *RequestThread(void *void_sln);
		static void TimedRequest(void *void_sln);

	public:
		SLN(EOServer *server);
		void Request();

	static SLN *ScriptFactory(EOServer *server) { return new SLN(server); }

	SCRIPT_REGISTER_REF(SLN)
		SCRIPT_REGISTER_FACTORY("SLN @f(EOServer @server)", ScriptFactory);
	SCRIPT_REGISTER_END()
};

#endif //SLN_HPP_INCLUDED
