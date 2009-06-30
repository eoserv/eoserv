
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef SLN_HPP_INCLUDED
#define SLN_HPP_INCLUDED

class SLN;

#include "eoserver.hpp"

/**
 * Manages checking in with the SLN server regularly
 */
class SLN
{
	private:
		EOServer *server;

		void RequestTick();
		static void *RequestThread(void *void_sln);
		static void TimedRequest(void *void_sln);

	public:
		SLN(EOServer *server);
		void Request();
};

#endif //SLN_HPP_INCLUDED
