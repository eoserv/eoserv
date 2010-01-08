
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef HANDLERS_H_INCLUDED
#define HANDLERS_H_INCLUDED

#include "stdafx.h"

#include "character.hpp"
#include "eoclient.hpp"
#include "eoserver.hpp"
#include "packet.hpp"
#include "player.hpp"
#include "world.hpp"

#ifdef CLIENT_F_FUNC
#undef CLIENT_F_FUNC
#endif // CLIENT_F_FUNC

#define CLIENT_F_FUNC(FUNC) bool EOClient::Handle_##FUNC(PacketFamily family, PacketAction action, PacketReader &reader, int act)

#define CLIENT_SENDRAW(REPLY) this->Send(REPLY)
#define CLIENT_SEND(REPLY) this->Send(this->processor.Encode(REPLY))
#define CLIENT_QUEUE_ACTION(time) if (!act) { this->queue.AddAction(family, action, reader, time); return true; }
#define CLIENT_FORCE_QUEUE_ACTION(time) { this->queue.AddAction(family, action, reader, time); return true; }

#endif // HANDLERS_H_INCLUDED
