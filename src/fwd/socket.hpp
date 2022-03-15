/* fwd/socket.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef FWD_SOCKET_HPP_INCLUDED
#define FWD_SOCKET_HPP_INCLUDED

class IPAddress;
class Client;
class Server;

/**
 * Return the OS last error message
 */
const char *OSErrorString();

#endif // FWD_SOCKET_HPP_INCLUDED
