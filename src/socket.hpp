#ifndef SOCKET_H_INCLUDED
#define SOCKET_H_INCLUDED

#include <string>
#include <list>
#include <stdint.h>
#include <cstddef>
#include <stdexcept>

#ifdef WIN32
#include <winsock2.h>
/**
 * Stores the initialization state of WinSock.
 */
extern bool ws_init;

/**
 * Stores internal WinSock information.
 */
extern WSADATA wsadata;

/**
 * Type for storing the size of a POSIX sockaddr_in struct.
 * Defined here because it does not exist in MinGW's winsock headers.
 */
typedef int socklen_t;
#else // WIN32
// Stop doxygen generating a gigantic include graph
#ifndef DOXYGEN
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#endif // DOXYGEN

/**
 * Type for storing a socket handle.
 * Defined here because it does not exist in linux headers.
 */
typedef int SOCKET;

/**
 * Socket handle representing an invalid socket.
 */
const SOCKET INVALID_SOCKET = -1;

/**
 * Return code representing a socket error.
 */
const int SOCKET_ERROR = -1;
#endif // WIN32

class IPAddress;
class Client;
template <class> class Server;

/**
 * Stores an IP address and converts between string and numeric formats.
 */
class IPAddress
{
	protected:
		/**
		 * Integer version of the IP address.
		 */
		uint32_t address;

		/**
		 * Return the IP address as a string (eg 255.255.255.255).
		 */
		std::string GetString();

	public:
		/**
		 * Initialize the address as 0.0.0.0.
		 */
		IPAddress();

		/**
		 * Initialize the address to the integer value.
		 */
		IPAddress(uint32_t);

		/**
		 * Initialize the address using 4 octets.
		 */
		IPAddress(uint8_t, uint8_t, uint8_t, uint8_t);

		/**
		 * Initialize the address using a string (eg 255.255.255.255).
		 */
		IPAddress(const char *);

		/**
		 * Initialize the address using a POSIX in_addr struct.
		 */
		IPAddress(in_addr);

		/**
		 * Initialize the address using a string (eg 255.255.255.255).
		 */
		IPAddress(std::string);

		/**
		 * Set the address to an integer value
		 */
		IPAddress &operator =(uint32_t);

		/**
		 * Set the address using a string (eg 255.255.255.255).
		 */
		IPAddress &operator =(const char *);

		/**
		 * Set the address using a POSIX in_addr struct.
		 */
		IPAddress &operator =(in_addr);

		/**
		 * Set the address using a string (eg 255.255.255.255).
		 */
		IPAddress &operator =(std::string);

		/**
		 * Return the IP address as an integer.
		 */
		operator uint32_t();

		/**
		 * Return the IP address as a POSIX in_addr struct.
		 */
		operator in_addr();

		/**
		 * Return the IP address as a string (eg 255.255.255.255).
		 */
		operator std::string();
};

/**
 * Generic TCP server class.
 */
template <class T = Client> class Server
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
		
		/**
		 * Stores file descriptors for Select().
		 */
		fd_set read_fds;
		
		/**
		 * Stores file descriptors for Select().
		 */
		fd_set write_fds;
		
		/**
		 * Stores file descriptors for Select().
		 */
		fd_set except_fds;
		
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

	private:
		/**
		 * Initializes all data and WinSock if required.
		 * This is called by every Socket constructor.
		 */
		void Initialize()
		{
#ifdef WIN32
			if (!ws_init)
			{
				if (WSAStartup(MAKEWORD(2,0), &wsadata) != 0)
				{
					this->state = Invalid;
					return;
				}
				ws_init = true;
			}
#endif // WIN32
			this->server = socket(AF_INET, SOCK_STREAM, 0);
			this->state = Created;
			this->recv_buffer_max = 1024*128;
			this->send_buffer_max = 1024*128;
			this->maxconn = 0;
		}

	protected:
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
		 * List of connected clients.
		 */
		std::list<T *> clients;

		/**
		 * Socket handle of the listener.
		 */
		SOCKET server;

		/**
		 * Current server state.
		 * @sa State
		 */
		State state;

	public:
		/**
		 * Initializes the Server.
		 */
		Server()
		{
			this->Initialize();
		}

		/**
		 * Initializes the Server and binds to the specified address and port.
		 * @param addr Address to bind to
		 * @param port Port number to bind to
		 */
		Server(IPAddress addr, uint16_t port)
		{
			this->Initialize();
			this->Bind(addr, port);
		}

		/**
		 * Bind the Server to the specified address and port.
		 * Once this succeeds you should call Listen().
		 * @param addr Address to bind to.
		 * @param port Port number to bind to.
		 * @return True if bind() was successful, false otherwise.
		 */
		bool Bind(IPAddress addr, uint16_t port)
		{
			sockaddr_in sin;
			this->address = addr;
			this->port = port;
			this->portn = htons(port);

			memset(&sin, 0, sizeof(sin));
			sin.sin_family = AF_INET;
			sin.sin_addr.s_addr = this->address;
			sin.sin_port = this->portn;

			if (bind(this->server, (sockaddr *)&sin, sizeof(sin)) == SOCKET_ERROR)
			{
				this->state = Invalid;
				return false;
			}

			this->state = Bound;
			return true;
		}

		/**
		 * Bind the Server to the specified address and port.
		 * @param maxconn Maximum number of clients to have at one time.
		 * @param backlog Number of connections to keep in the queue.
		 * @return True if listen() was successful, false otherwise.
		 */
		bool Listen(int maxconn, int backlog = 10)
		{
			this->maxconn = maxconn;

			if (this->state == Bound)
			{
				if (listen(this->server, maxconn) != SOCKET_ERROR)
				{
					this->state = Listening;
					return true;
				}
			}

			this->state = Invalid;
			return false;
		}

		/**
		 * Check for new connection requests.
		 * @return NULL if there are no pending connections, a pointer to the Client otherwise.
		 */
		T *Poll()
		{
			SOCKET newsock;
			sockaddr_in sin;
			socklen_t addrsize = sizeof(sockaddr_in);
			T* newclient;
#ifdef WIN32
			unsigned long nonblocking;
#endif // WIN32

			if (this->clients.size() >= this->maxconn)
			{
				if ((newsock = accept(this->server, (sockaddr *)&sin, &addrsize)) != INVALID_SOCKET)
				{
#ifdef WIN32
					closesocket(newsock);
#else // WIN32
					close(newsock);
#endif // WIN32
					return NULL;
				}
			}

#ifdef WIN32
			nonblocking = 1;
			ioctlsocket(this->server, FIONBIO, &nonblocking);
#else // WIN32
			fcntl(this->server, F_SETFL, FNONBLOCK|FASYNC);
#endif // WIN32
			if ((newsock = accept(this->server, (sockaddr *)&sin, &addrsize)) == INVALID_SOCKET)
			{
				return NULL;
			}
#ifdef WIN32
			nonblocking = 0;
			ioctlsocket(this->server, FIONBIO, &nonblocking);
#else // WIN32
			fcntl(this->server, F_SETFL, 0);
#endif // WIN32

			newclient = new T(newsock, sin, static_cast<void *>(this));
			newclient->send_buffer_max = this->send_buffer_max;

			this->clients.push_back(newclient);

			return newclient;
		}

		/**
		 * Check clients for incoming data and errors, and sends data in their send_buffer.
		 * If data is recieved, it is added to their recv_buffer.
		 * @return Returns a list of clients that have data in their recv_buffer.
		 */
		std::list<T *> Select(int timeout)
		{
			timeval timeout_val = {timeout/1000000, timeout%1000000};
			std::list<T *> selected;
			class std::list<T *>::iterator it;
			SOCKET nfds = this->server + 1;
			int result;

			FD_ZERO(&this->read_fds);
			FD_ZERO(&this->write_fds);
			FD_ZERO(&this->except_fds);

			for (it = this->clients.begin(); it != this->clients.end(); ++it)
			{
				if ((*it)->connected)
				{
					if ((*it)->recv_buffer.length() < this->recv_buffer_max)
					{
						FD_SET((*it)->sock, &this->read_fds);
					}

					if ((*it)->send_buffer.length() > 0)
					{
						FD_SET((*it)->sock, &this->write_fds);
					}

					FD_SET((*it)->sock, &this->except_fds);

					if ((*it)->sock + 1 > nfds)
					{
						nfds = (*it)->sock + 1;
					}
				}
			}

			FD_SET(this->server, &this->except_fds);

			result = select(nfds, &this->read_fds, &this->write_fds, &this->except_fds, &timeout_val);

			if (result == -1)
			{
				throw std::runtime_error("There was an error calling select().");
			}

			if (result > 0)
			{
				if (FD_ISSET(this->server, &this->except_fds))
				{
					throw std::runtime_error("There was an exception on the listening socket.");
				}

				for (it = this->clients.begin(); it != this->clients.end(); ++it)
				{
					if (FD_ISSET((*it)->sock, &this->except_fds))
					{
						(*it)->connected = false;
						continue;
					}

					if (FD_ISSET((*it)->sock, &this->read_fds))
					{
						char buf[32767];
						int recieved = recv((*it)->sock, buf, 32767, 0);
						if (recieved > 0)
						{
							buf[recieved] = '\0';
							(*it)->recv_buffer.append(buf, recieved);
						}
						else
						{
							(*it)->connected = false;
							continue;
						}

						if ((*it)->recv_buffer.length() > this->recv_buffer_max)
						{
							(*it)->connected = false;
							continue;
						}
					}

					if (FD_ISSET((*it)->sock, &this->write_fds))
					{
						int written = send((*it)->sock, (*it)->send_buffer.c_str(), (*it)->send_buffer.length(), 0);
						if (written == SOCKET_ERROR)
						{
							(*it)->connected = false;
							continue;
						}
						(*it)->send_buffer.erase(0,written);
					}
				}
			}

			for (it = this->clients.begin(); it != this->clients.end(); ++it)
			{
				if ((*it)->recv_buffer.length() > 0)
				{
					selected.push_back(*it);
				}
			}

			return selected;
		}

		/**
		 * Destroys any dead clients, should be called periodically.
		 * All pointers to Client objects from this Server should be considered invalid after execution.
		 */
		void BuryTheDead()
		{
			class std::list<T *>::iterator it;

			for (it = this->clients.begin(); it != this->clients.end(); ++it)
			{
				if (!(*it)->Connected())
				{
					delete *it;
					it = this->clients.erase(it);
				}
			}
		}

		State State()
		{
			return this->state;
		}

		int Connections()
		{
			return this->clients.size();
		}

		int MaxConnections()
		{
			return this->maxconn;
		}
};

/**
 * Generic TCP client class.
 */
class Client
{
	private:
		Client();

	protected:
		bool connected;
		SOCKET sock;
		sockaddr_in sin;
		std::string send_buffer;
		std::string recv_buffer;
		std::size_t send_buffer_max;
		void *server;

	public:
		Client(void *);
		Client(SOCKET, sockaddr_in, void *);
		std::string Recv(std::size_t length);
		void Send(const std::string &data);
		bool Connected();
		IPAddress GetRemoteAddr();
		bool Close();
		virtual ~Client();

	template<class> friend class Server;
};


#endif // SOCKET_H_INCLUDED
