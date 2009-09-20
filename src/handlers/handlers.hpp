
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include <algorithm>
#include <string>
#include <cstdio>
#include <cmath>
#include <list>
#include <vector>

#include "eoclient.hpp"
#include "eoserver.hpp"
#include "world.hpp"
#include "player.hpp"
#include "character.hpp"
#include "party.hpp"
#include "guild.hpp"
#include "packet.hpp"
#include "eoconst.hpp"
#include "eodata.hpp"
#include "util.hpp"
#include "console.hpp"

#ifdef CLIENT_F_FUNC
#undef CLIENT_F_FUNC
#endif // CLIENT_F_FUNC

#define CLIENT_F_FUNC(FUNC) bool EOClient::Handle_##FUNC(PacketFamily family, PacketAction action, PacketReader &reader, int act)

#define CLIENT_SENDRAW(REPLY) this->Send(REPLY)
#define CLIENT_SEND(REPLY) this->Send(this->processor.Encode(REPLY))
#define CLIENT_QUEUE_ACTION(time) if (!act) { this->queue.push(new ActionQueue_Action(family, action, reader, time)); return true; }
