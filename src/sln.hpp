
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef SLN_HPP_INCLUDED
#define SLN_HPP_INCLUDED

#include <pthread.h>

class SLN;

#include "eoserver.hpp"

/**
 * Manages checking in with the SLN server regularly
 */
class SLN
{
	private:
		pthread_t thread;
		EOServer *server;

		void RequestTick();
		static void *RequestThread(void *void_sln);
		static void TimedRequest(void *void_sln);

	public:
		SLN(EOServer *server);
		void Request();
		~SLN();
};

#endif //SLN_HPP_INCLUDED
