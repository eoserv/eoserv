
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef SLN_HPP_INCLUDED
#define SLN_HPP_INCLUDED

#include "nanohttp.hpp"
#include "timer.hpp"

namespace SLN
{

extern HTTP *http;
extern TimeEvent *tick_request_timer;

void request(void *server_void);
void *real_request(void *server_void);
void tick_request(void *server_void);

}

#endif //SLN_HPP_INCLUDED
