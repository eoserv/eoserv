
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "handlers.hpp"

namespace Handlers
{

// User requests another's Book (Quest history)
void Book_List(Character *character, PacketReader &reader)
{
	(void)character;
	(void)reader;
}

PACKET_HANDLER_REGISTER(PACKET_BOOK)
	// Register(PACKET_LIST, Book_List, Playing);
PACKET_HANDLER_REGISTER_END()

}
