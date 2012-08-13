
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef SOCKET_HPP_INCLUDED
#define SOCKET_HPP_INCLUDED

#include "fwd/socket.hpp"

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <exception>
#include <list>
#include <memory>
#include <string>
#include <cstdint>
#include <unordered_map>
#include <vector>

/**
 * Generic Socket exception type
 */
class Socket_Exception : public std::exception
{
	protected:
		const char *err;
	public:
		Socket_Exception(const char *e) : err(e) {};
		const char *error()  const noexcept { return err; };
		virtual const char *what() const noexcept { return "Socket_Exception"; }
};

/**
 * Exception thrown when intializing the socket library failed
 */
class Socket_InitFailed : public Socket_Exception
{
	public:
		Socket_InitFailed(const char *e) : Socket_Exception(e) {}
		const char *what() const noexcept { return "Socket_InitFailed"; }
};

/**
 * Exception thrown when a call to bind() failed
 */
class Socket_BindFailed : public Socket_Exception
{
	public:
		Socket_BindFailed(const char *e) : Socket_Exception(e) {}
		const char *what() const noexcept { return "Socket_BindFailed"; }
};

/**
 * Exception thrown when a call to listen() failed
 */
class Socket_ListenFailed : public Socket_Exception
{
	public:
		Socket_ListenFailed(const char *e) : Socket_Exception(e) {}
		const char *what() const noexcept { return "Socket_ListenFailed"; }
};

/**
 * Exception thrown when a call to select() failed
 */
class Socket_SelectFailed : public Socket_Exception
{
	public:
		Socket_SelectFailed(const char *e) : Socket_Exception(e) {}
		const char *what() const noexcept { return "Socket_SelectFailed"; }
};

/**
 * Static initialization helper for socket subsystem
 */
struct Socket_Init
{
	Socket_Init()
	{
		static bool initialized = false;

		if (!initialized)
		{
			initialized = true;
			init();
		}
	}

	void init();
};

/**
 * Stores an IP address and converts between string and numeric formats.
 */
class IPAddress
{
	protected:
		/**
		 * Integer version of the IP address.
		 */
		std::uint32_t address;

	public:
		/**
		 * Initialize the address as 0.0.0.0.
		 */
		IPAddress();

		/**
		 * Initialize the address to the integer value.
		 */
		IPAddress(unsigned int);

		/**
		 * Initialize the address using 4 octets.
		 */
		IPAddress(unsigned char, unsigned char, unsigned char, unsigned char);

		/**
		 * Initialize the address using a string (eg 255.255.255.255).
		 */
		IPAddress(const char *);

		/**
		 * Initialize the address using a string (eg 255.255.255.255).
		 * Only accepts IP addresses, see Lookup to lookup hostnames
		 */
		IPAddress(std::string);

		/**
		 * Lookup a hostname and use that to create an IPAddress class
		 */
		static IPAddress Lookup(std::string host);

		/**
		 * Set the address to an integer value
		 */
		IPAddress &SetInt(unsigned int);

		/**
		 * Set the address using 4 octets.
		 */
		IPAddress &SetOctets(unsigned char, unsigned char, unsigned char, unsigned char);

		/**
		 * Set the address using a string (eg 255.255.255.255).
		 */
		IPAddress &SetString(const char *);

		/**
		 * Set the address using a string (eg 255.255.255.255).
		 */
		IPAddress &SetString(std::string);

		/**
		 * Set the address to an integer value
		 */
		IPAddress &operator =(unsigned int);

		/**
		 * Set the address using a string (eg 255.255.255.255).
		 */
		IPAddress &operator =(const char *);

		/**
		 * Set the address using a string (eg 255.255.255.255).
		 */
		IPAddress &operator =(std::string);

		/**
		 * Return the IP address as an integer.
		 */
		unsigned int GetInt() const;

		/**
		 * Return the IP address as a string (eg 255.255.255.255).
		 */
		std::string GetString() const;

		/**
		 * Return the IP address as an integer.
		 */
		operator unsigned int() const;

		/**
		 * Return the IP address as a string (eg 255.255.255.255).
		 */
		operator std::string() const;

		bool operator ==(const IPAddress &) const;
};

namespace std
{
	template <> struct hash<IPAddress> : public unary_function<IPAddress, std::size_t>
	{
		std::size_t operator()(const IPAddress &ipaddress) const
		{
			return ipaddress.GetInt();
		}
	};
}

// Temporary
// TODO: Merge Client and Server with Socket

struct Socket;

/**
 * Generic TCP client class.
 */
class Client
{
	private:
		struct impl_;
		std::auto_ptr<impl_> impl;

	protected:
		Server *server;
		bool connected;
		std::time_t closed_time;
		std::time_t connect_time;

		std::string recv_buffer;
		std::size_t recv_buffer_gpos;
		std::size_t recv_buffer_ppos;
		std::size_t recv_buffer_used;

		std::string send_buffer;
		std::size_t send_buffer_gpos;
		std::size_t send_buffer_ppos;
		std::size_t send_buffer_used;

	public:
		Client();
		Client(const IPAddress &addr, std::uint16_t port);
		Client(Server *);
		Client(const Socket &, Server *);

		virtual bool NeedTick() { return false; }

		void SetRecvBuffer(std::size_t size);
		void SetSendBuffer(std::size_t size);

		bool Connect(const IPAddress &addr, std::uint16_t port);
		void Bind(const IPAddress &addr, std::uint16_t port);

		std::size_t RecvBufferRemaining() { return this->recv_buffer.length() - this->recv_buffer_used; }
		std::size_t SendBufferRemaining() { return this->send_buffer.length() - this->send_buffer_used; }

		std::string Recv(std::size_t length);
		void Send(const std::string &data);

		bool DoRecv();
		bool DoSend();

		bool Select(double timeout);

		bool Connected() const;
		IPAddress GetRemoteAddr() const;

		void Close(bool force = false);

		std::time_t ConnectTime() const;

		virtual ~Client();

	// TODO: Separate Socket type
	friend class Server;
};

/**
 * Generic TCP server class.
 */
class Server
{
	public:
		enum State
		{
			/**
			 * There was an error preparing the server.
			 */
			Invalid,

			/**
			 * Newly created server, not listening yet.
			 */
			Created,

			/**
			 * Server has been bound to a port but is not yet listening.
			 */
			Bound,

			/**
			 * Server is listening and is ready to accept clients.
			 */
			Listening
		};

	private:
		struct impl_;

		impl_ *impl;

	protected:
		virtual Client *ClientFactory(const Socket &sock) { return new Client(sock, this); }

		/**
		 * The address the server will listen on.
		 */
		IPAddress address;

		/**
		 * The port the server will listen on in host order.
		 */
		uint16_t port;

		/**
		 * The port the server will listen on in network order.
		 */
		uint16_t portn;

		/**
		 * Current server state.
		 * @sa State
		 */
		State state;

		/**
		 * Maximum amount of data that will be buffered for recieving per client.
		 */
		std::size_t recv_buffer_max;

		/**
		 * Maximum amount of data that will be buffered for sending per client.
		 */
		std::size_t send_buffer_max;

		/**
		 * Maximum number of connections the server will hold at one time.
		 */
		unsigned int maxconn;

	public:
		/**
		 * List of connected clients.
		 */
		std::list<Client *> clients;

		/**
		 * Initializes the Server.
		 */
		Server();

		/**
		 * Initializes the Server and binds to the specified address and port.
		 * @param addr Address to bind to
		 * @param port Port number to bind to
		 */
		Server(const IPAddress &addr, uint16_t port);

		/**
		 * Bind the Server to the specified address and port.
		 * Once this succeeds you should call Listen().
		 * @param addr Address to bind to.
		 * @param port Port number to bind to.
		 * @throw Socket_BindFailed
		 */
		void Bind(const IPAddress &addr, uint16_t port);

		/**
		 * Bind the Server to the specified address and port.
		 * @param maxconn Maximum number of clients to have at one time.
		 * @param backlog Number of connections to keep in the queue.
		 * @throw Socket_ListenFailed
		 */
		void Listen(int maxconn, int backlog = 10);

		/**
		 * Check for new connection requests.
		 * @return NULL if there are no pending connections, a pointer to the Client otherwise.
		 */
		Client *Poll();

		/**
		 * Check clients for incoming data and errors, and sends data in their send_buffer.
		 * If data is recieved, it is added to their recv_buffer.
		 * @param timeout Max number of seconds to block for
		 * @throw Socket_SelectFailed
		 * @throw Socket_Exception
		 * @return Returns a list of clients that have data in their recv_buffer.
		 */
		std::vector<Client *> *Select(double timeout);

		/**
		 * Destroys any dead clients, should be called periodically.
		 * All pointers to Client objects from this Server should be considered invalid after execution.
		 */
		void BuryTheDead();

		State State() const
		{
			return this->state;
		}

		int Connections() const
		{
			return this->clients.size();
		}

		int MaxConnections() const
		{
			return this->maxconn;
		}

		virtual ~Server();
};


#endif // SOCKET_HPP_INCLUDED
