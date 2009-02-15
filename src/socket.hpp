#ifndef SOCKET_H_INCLUDED
#define SOCKET_H_INCLUDED

#include <string>
#include <list>
#include <stdint.h>
#include <cstddef>
#include <stdexcept>

#ifdef WIN32
#include <winsock2.h>
extern bool ws_init;
extern WSADATA wsadata;
typedef int socklen_t;
#else // WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
typedef int SOCKET;
const SOCKET INVALID_SOCKET = -1;
const int SOCKET_ERROR = -1;
#endif // WIN32

class IPAddress;
class Client;
template <class> class Server;

class IPAddress
{
	protected:
		uint32_t address;

		std::string GetString();

	public:
		IPAddress();
		IPAddress(uint32_t);
		IPAddress(uint8_t, uint8_t, uint8_t, uint8_t);
		IPAddress(const char *);
		IPAddress(in_addr);
		IPAddress(std::string);

		IPAddress &operator =(uint32_t);
		IPAddress &operator =(const char *);
		IPAddress &operator =(in_addr);
		IPAddress &operator =(std::string);

		operator uint32_t();
		operator in_addr();
		operator std::string();
};

template <class T = Client> class Server
{
	public:
		enum State
		{
			Invalid,
			Created,
			Bound,
			Listening
		};
		fd_set read_fds;
		fd_set write_fds;
		fd_set except_fds;
		std::size_t recv_buffer_max;
		std::size_t send_buffer_max;
		unsigned int maxconn;

	private:
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
		IPAddress address;
		uint16_t port;
		uint16_t portn;
		std::list<T *> clients;
		SOCKET server;
		State state;

	public:
		Server()
		{
			this->Initialize();
		}

		Server(IPAddress addr, uint16_t port)
		{
			this->Initialize();
			this->Bind(addr, port);
		}

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

			newclient = new T(newsock, sin);
			newclient->send_buffer_max = this->send_buffer_max;
			newclient->server = static_cast<void *>(this);

			this->clients.push_back(newclient);

			return newclient;
		}

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

		// Destroys any dead clients, should be called periodically
		// WARNING: All pointers to Client objects from this server
		// should be considered invalid after execution.
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

class Client
{
	protected:
		bool connected;
		SOCKET sock;
		sockaddr_in sin;
		std::string send_buffer;
		std::string recv_buffer;
		std::size_t send_buffer_max;
		void *server;

	public:
		Client();
		Client(SOCKET, sockaddr_in);
		std::string Recv(std::size_t length);
		void Send(const std::string &data);
		bool Connected();
		IPAddress GetRemoteAddr();
		bool Close();
		virtual ~Client();

	template<class> friend class Server;
};


#endif // SOCKET_H_INCLUDED
