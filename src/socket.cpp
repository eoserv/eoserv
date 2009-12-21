
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "socket.hpp"

#include <cerrno>

static char ErrorBuf[1024];

#if defined(WIN32) || defined(WIN64)
bool socket_ws_init = false;
WSADATA socket_wsadata;

const char *OSErrorString()
{
	int error = GetLastError();

	if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, ErrorBuf, 1023, 0))
	{
		snprintf(ErrorBuf, 1024, "Unknown error %i", error);
	}
	else
	{
		strncpy(ErrorBuf, util::trim(ErrorBuf).c_str(), 1023);
	}

	return ErrorBuf;
}
#else // defined(WIN32) || defined(WIN64)
#include <cerrno>
#include <string.h>
const char *OSErrorString()
{
	strerror_r(errno, ErrorBuf, 1024);
	return ErrorBuf;
}
#endif // defined(WIN32) || defined(WIN64)

IPAddress::IPAddress()
{
	Socket_WSAStartup();
	this->SetInt(0);
}

IPAddress::IPAddress(std::string str_addr)
{
	Socket_WSAStartup();
	this->SetString(str_addr);
}

IPAddress::IPAddress(const char *str_addr)
{
	Socket_WSAStartup();
	this->SetString(str_addr);
}

IPAddress::IPAddress(unsigned int addr)
{
	Socket_WSAStartup();
	this->SetInt(addr);
}

IPAddress::IPAddress(in_addr addr)
{
	Socket_WSAStartup();
	this->SetInt(ntohl(addr.s_addr));
}

IPAddress::IPAddress(unsigned char o1, unsigned char o2, unsigned char o3, unsigned char o4)
{
	Socket_WSAStartup();
	this->SetOctets(o1, o2, o3, o4);
}

IPAddress IPAddress::Lookup(std::string host)
{
	Socket_WSAStartup();
	addrinfo hints;
	addrinfo *ai;
	IPAddress ipaddr;

	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if (getaddrinfo(host.c_str(), 0, &hints, &ai) != 0)
	{
		ipaddr.address = 0;
	}
	else
	{
		ipaddr.address = ntohl(reinterpret_cast<sockaddr_in *>(ai->ai_addr)->sin_addr.s_addr);
	}

	freeaddrinfo(ai);

	return ipaddr;
}

IPAddress &IPAddress::SetInt(unsigned int addr)
{
	this->address = addr;
	return *this;
}

IPAddress &IPAddress::SetOctets(unsigned char o1, unsigned char o2, unsigned char o3, unsigned char o4)
{
	this->address = o1 << 24 | o2 << 16 | o3 << 8 | o4;
	return *this;
}

IPAddress &IPAddress::SetString(const char *str_addr)
{
	return this->SetString(std::string(str_addr));
}

IPAddress &IPAddress::SetString(std::string str_addr)
{
	unsigned int io1, io2, io3, io4;
	std::sscanf(str_addr.c_str(), "%u.%u.%u.%u", &io1, &io2, &io3, &io4);
	unsigned char o1 = io1, o2 = io2, o3 = io3, o4 = io4;
	this->address = o1 << 24 | o2 << 16 | o3 << 8 | o4;
	return *this;
}

IPAddress &IPAddress::operator =(std::string str_addr)
{
	return this->SetString(str_addr);
}

IPAddress &IPAddress::operator =(const char *str_addr)
{
	return this->SetString(str_addr);
}

IPAddress &IPAddress::operator =(unsigned int addr)
{
	return this->SetInt(addr);
}

unsigned int IPAddress::GetInt() const
{
	return this->address;
}

std::string IPAddress::GetString() const
{
	char buf[16];
	unsigned char o1, o2, o3, o4;
	o1 = (this->address & 0xFF000000) >> 24;
	o2 = (this->address & 0x00FF0000) >> 16;
	o3 = (this->address & 0x0000FF00) >> 8;
	o4 = this->address & 0x000000FF;
	std::sprintf(buf, "%u.%u.%u.%u", o1, o2, o3, o4);
	return std::string(buf);
}

IPAddress::operator unsigned int() const
{
	return this->address;
}

IPAddress::operator in_addr() const
{
	in_addr addr;
	addr.s_addr = htonl(this->address);
	return addr;
}

IPAddress::operator std::string() const
{
	return this->GetString();
}

bool IPAddress::operator ==(const IPAddress &other) const
{
	return (this->address == other.address);
}

bool IPAddress::operator <(const IPAddress &other) const
{
	return (this->address < other.address);
}


Client::Client()
{
	Socket_WSAStartup();
	this->connected = false;
	this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	this->send_buffer_max = static_cast<unsigned int>(-1);
	this->recv_buffer_max = static_cast<unsigned int>(-1);
}

Client::Client(const IPAddress &addr, uint16_t port)
{
	Socket_WSAStartup();
	this->connected = false;
	this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	this->send_buffer_max = static_cast<unsigned int>(-1);
	this->recv_buffer_max = static_cast<unsigned int>(-1);

	this->Connect(addr, port);
}

Client::Client(Server *server)
{
	Socket_WSAStartup();
	this->connected = false;
	this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	this->server = server;
}

Client::Client(SOCKET sock, sockaddr_in sin, Server *server)
{
	Socket_WSAStartup();
	this->connected = true;
	this->sock = sock;
	this->sin = sin;
	this->server = server;
}

bool Client::Connect(const IPAddress &addr, uint16_t port)
{
	std::memset(&this->sin, 0, sizeof(this->sin));
	this->sin.sin_family = AF_INET;
	this->sin.sin_addr = addr;
	this->sin.sin_port = htons(port);

	if (connect(this->sock, reinterpret_cast<sockaddr *>(&this->sin), sizeof(this->sin)) != 0)
	{
		return this->connected = false;
	}

	return this->connected = true;
}

void Client::Bind(const IPAddress &addr, uint16_t port)
{
	sockaddr_in sin;
	uint16_t portn = htons(port);

	std::memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr = addr;
	sin.sin_port = portn;

	if (bind(this->sock, reinterpret_cast<sockaddr *>(&sin), sizeof(sin)) == SOCKET_ERROR)
	{
		throw Socket_BindFailed(OSErrorString());
	}
}

std::string Client::Recv(std::size_t length)
{
	if (length <= 0)
	{
		return "";
	}
	std::string ret = this->recv_buffer.substr(0, length);
	this->recv_buffer.erase(0, length);
	return ret;
}

void Client::Send(const std::string &data)
{
	this->send_buffer += data;

	if (this->send_buffer.length() > this->send_buffer_max)
	{
		this->connected = false;
	}
}

#if defined(SOCKET_POLL) && !defined(WIN32) && !defined(WIN64)
void Client::Tick(double timeout)
{
	pollfd fd;

	fd.fd = this->sock;
	fd.events = POLLIN;

	if (this->send_buffer.length() > 0)
	{
		fd.events |= POLLOUT;
	}

	int result = poll(&fd, 1, long(timeout * 1000));

	if (result == -1)
	{
		throw Socket_SelectFailed(OSErrorString());
	}

	if (result > 0)
	{
		if (fd.revents & POLLERR || fd.revents & POLLHUP || fd.revents & POLLNVAL)
		{
			this->Close();
			return;
		}

		if (fd.revents & POLLIN)
		{
			char buf[32767];
			int recieved = recv(this->sock, buf, 32767, 0);
			if (recieved > 0)
			{
				this->recv_buffer.append(buf, recieved);
			}
			else
			{
				this->Close();
				return;
			}

			if (this->recv_buffer.length() > this->recv_buffer_max)
			{
				this->Close();
				return;
			}
		}

		if (fd.revents & POLLOUT)
		{
			int written = send(this->sock, this->send_buffer.c_str(), this->send_buffer.length(), 0);
			if (written == SOCKET_ERROR)
			{
				this->Close();
				return;
			}
			this->send_buffer.erase(0,written);
		}
	}
}
#else // defined(SOCKET_POLL) && !defined(WIN32) && !defined(WIN64)
void Client::Tick(double timeout)
{
	fd_set read_fds, write_fds, except_fds;
	long tsecs = long(timeout);
	timeval timeout_val = {tsecs, long((timeout - double(tsecs))*1000000)};

	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);
	FD_ZERO(&except_fds);

	if (this->recv_buffer.length() < this->recv_buffer_max)
	{
		FD_SET(this->sock, &read_fds);
	}

	if (this->send_buffer.length() > 0)
	{
		FD_SET(this->sock, &write_fds);
	}

	FD_SET(this->sock, &except_fds);

	int result = select(this->sock+1, &read_fds, &write_fds, &except_fds, &timeout_val);
	if (result == -1)
	{
		throw Socket_SelectFailed(OSErrorString());
	}

	if (result > 0)
	{
		if (FD_ISSET(this->sock, &except_fds))
		{
			this->Close();
			return;
		}

		if (FD_ISSET(this->sock, &read_fds))
		{
			char buf[32767];
			int recieved = recv(this->sock, buf, 32767, 0);
			if (recieved > 0)
			{
				this->recv_buffer.append(buf, recieved);
			}
			else
			{
				this->Close();
				return;
			}

			if (this->recv_buffer.length() > this->recv_buffer_max)
			{
				this->Close();
				return;
			}
		}

		if (FD_ISSET(this->sock, &write_fds))
		{
			int written = send(this->sock, this->send_buffer.c_str(), this->send_buffer.length(), 0);
			if (written == SOCKET_ERROR)
			{
				this->Close();
				return;
			}
			this->send_buffer.erase(0,written);
		}
	}
}
#endif // defined(SOCKET_POLL) && !defined(WIN32) && !defined(WIN64)

bool Client::Connected()
{
	return this->connected;
}

void Client::Close(bool force)
{
	if (this->connected)
	{
		this->connected = false;
		this->closed_time = std::time(0);

		// This won't work properly for the first 2 seconds of 1 January 1970
		if (force)
		{
			this->closed_time = 0;
		}
	}
}

IPAddress Client::GetRemoteAddr()
{
	return IPAddress(this->sin.sin_addr);
}

time_t Client::ConnectTime()
{
	return this->connect_time;
}

Client::~Client()
{
	if (this->connected)
	{
		this->Close();
	}
}


void Server::Initialize()
{
#if defined(WIN32) || defined(WIN64)
	if (!socket_ws_init)
	{
		if (WSAStartup(MAKEWORD(2,0), &socket_wsadata) != 0)
		{
			this->state = Invalid;
			throw Socket_InitFailed(OSErrorString());
		}
		socket_ws_init = true;
	}
#endif // defined(WIN32) || defined(WIN64)
	this->server = socket(AF_INET, SOCK_STREAM, 0);
	this->state = Created;
	this->recv_buffer_max = 1024*128;
	this->send_buffer_max = 1024*128;
	this->maxconn = 0;
}

Client *Server::ClientFactory(SOCKET sock, sockaddr_in sin)
{
	return new Client(sock, sin, this);
}

void Server::Bind(const IPAddress &addr, uint16_t port)
{
	sockaddr_in sin;
	this->address = addr;
	this->port = port;
	this->portn = htons(port);

	std::memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr = this->address;
	sin.sin_port = this->portn;

	if (bind(this->server, reinterpret_cast<sockaddr *>(&sin), sizeof(sin)) == SOCKET_ERROR)
	{
		this->state = Invalid;
		throw Socket_BindFailed(OSErrorString());
	}

	this->state = Bound;
}

void Server::Listen(int maxconn, int backlog)
{
	this->maxconn = maxconn;

	//if (this->state == Bound)
	//{
		if (listen(this->server, backlog) != SOCKET_ERROR)
		{
			this->state = Listening;
			return;
		}
	//}

	this->state = Invalid;
	throw Socket_ListenFailed(OSErrorString());
}

Client *Server::Poll()
{
	SOCKET newsock;
	sockaddr_in sin;
	socklen_t addrsize = sizeof(sockaddr_in);
	Client *newclient;
#if defined(WIN32) || defined(WIN64)
	unsigned long nonblocking;
#endif // defined(WIN32) || defined(WIN64)

	if (this->clients.size() >= this->maxconn)
	{
		if ((newsock = accept(this->server, reinterpret_cast<sockaddr *>(&sin), &addrsize)) != INVALID_SOCKET)
		{
#if defined(WIN32) || defined(WIN64)
			closesocket(newsock);
#else // defined(WIN32) || defined(WIN64)
			close(newsock);
#endif // defined(WIN32) || defined(WIN64)
			return 0;
		}
	}

#if defined(WIN32) || defined(WIN64)
	nonblocking = 1;
	ioctlsocket(this->server, FIONBIO, &nonblocking);
#else // defined(WIN32) || defined(WIN64)
	fcntl(this->server, F_SETFL, FNONBLOCK|FASYNC);
#endif // defined(WIN32) || defined(WIN64)
	if ((newsock = accept(this->server, reinterpret_cast<sockaddr *>(&sin), &addrsize)) == INVALID_SOCKET)
	{
		return 0;
	}
#if defined(WIN32) || defined(WIN64)
	nonblocking = 0;
	ioctlsocket(this->server, FIONBIO, &nonblocking);
#else // defined(WIN32) || defined(WIN64)
	fcntl(this->server, F_SETFL, 0);
#endif // defined(WIN32) || defined(WIN64)

	newclient = this->ClientFactory(newsock, sin);
	newclient->send_buffer_max = this->send_buffer_max;
	newclient->recv_buffer_max = this->recv_buffer_max;
	newclient->connect_time = time(0);

	this->clients.push_back(newclient);

	return newclient;
}

#if defined(SOCKET_POLL) && !defined(WIN32) && !defined(WIN64)
PtrVector<Client> &Server::Select(double timeout)
{
	static PtrVector<Client> selected;
	std::vector<pollfd> fds;
	int result;
	pollfd fd;

	fds.reserve(this->clients.size() + 1);

	fd.fd = this->server;
	fd.events = POLLERR;
	fds.push_back(fd);

	UTIL_LIST_FOREACH_ALL(this->clients, Client, client)
	{
		fd.fd = client->sock;

		fd.events = 0;

		if (client->recv_buffer.length() < client->recv_buffer_max)
		{
			fd.events |= POLLIN;
		}

		if (client->send_buffer.length() > 0)
		{
			fd.events |= POLLOUT;
		}

		fds.push_back(fd);
	}

	result = poll(&fds[0], fds.size(), long(timeout * 1000));

	if (result == -1)
	{
		throw Socket_SelectFailed(OSErrorString());
	}

	if (result > 0)
	{
		if (fds[0].revents & POLLERR == POLLERR)
		{
			throw Socket_Exception("There was an exception on the listening socket.");
		}

		int i = 0;
		UTIL_PTR_LIST_FOREACH(this->clients, Client, client)
		{
			++i;
			if (fds[i].revents & POLLERR || fds[i].revents & POLLHUP || fds[i].revents & POLLNVAL)
			{
				client->Close();
				continue;
			}

			if (fds[i].revents & POLLIN)
			{
				char buf[32767];
				int recieved = recv(client->sock, buf, 32767, 0);
				if (recieved > 0)
				{
					client->recv_buffer.append(buf, recieved);
				}
				else
				{
					client->Close();
					continue;
				}

				if (client->recv_buffer.length() > client->recv_buffer_max)
				{
					client->Close();
					continue;
				}
			}

			if (fds[i].revents & POLLOUT)
			{
				int written = send(client->sock, client->send_buffer.c_str(), client->send_buffer.length(), 0);
				if (written == SOCKET_ERROR)
				{
					client->Close();
					continue;
				}
				client->send_buffer.erase(0,written);
			}
		}
	}

	UTIL_PTR_LIST_FOREACH(this->clients, Client, client)
	{
		if (client->connected || client->recv_buffer.length() > 0)
		{
			selected.push_back(client);
		}
	}

	return &selected;
}
#else // defined(SOCKET_POLL) && !defined(WIN32) && !defined(WIN64)
PtrVector<Client> *Server::Select(double timeout)
{
	long tsecs = long(timeout);
	timeval timeout_val = {tsecs, long((timeout - double(tsecs))*1000000)};
	static PtrVector<Client> selected;
	SOCKET nfds = this->server;
	int result;

	FD_ZERO(&this->read_fds);
	FD_ZERO(&this->write_fds);
	FD_ZERO(&this->except_fds);

	UTIL_PTR_LIST_FOREACH(this->clients, Client, client)
	{
		if (client->recv_buffer.length() < client->recv_buffer_max)
		{
			FD_SET(client->sock, &this->read_fds);
		}

		if (client->send_buffer.length() > 0)
		{
			FD_SET(client->sock, &this->write_fds);
		}

		FD_SET(client->sock, &this->except_fds);

		if (client->sock > nfds)
		{
			nfds = client->sock;
		}
	}

	FD_SET(this->server, &this->except_fds);

	result = select(nfds+1, &this->read_fds, &this->write_fds, &this->except_fds, &timeout_val);

	if (result == -1)
	{
		throw Socket_SelectFailed(OSErrorString());
	}

	if (result > 0)
	{
		if (FD_ISSET(this->server, &this->except_fds))
		{
			throw Socket_Exception("There was an exception on the listening socket.");
		}

		UTIL_PTR_LIST_FOREACH(this->clients, Client, client)
		{
			if (FD_ISSET(client->sock, &this->except_fds))
			{
				client->Close();
				continue;
			}

			if (FD_ISSET(client->sock, &this->read_fds))
			{
				char buf[32767];
				int recieved = recv(client->sock, buf, 32767, 0);
				if (recieved > 0)
				{
					client->recv_buffer.append(buf, recieved);
				}
				else
				{
					client->Close();
					continue;
				}

				if (client->recv_buffer.length() > client->recv_buffer_max)
				{
					client->Close();
					continue;
				}
			}

			if (FD_ISSET(client->sock, &this->write_fds))
			{
				int written = send(client->sock, client->send_buffer.c_str(), client->send_buffer.length(), 0);
				if (written == SOCKET_ERROR)
				{
					client->Close();
					continue;
				}
				client->send_buffer.erase(0,written);
			}
		}
	}

	UTIL_PTR_LIST_FOREACH(this->clients, Client, client)
	{
		if (client->recv_buffer.length() > 0)
		{
			selected.push_back(*client);
		}
	}

	return &selected;
}
#endif // defined(SOCKET_POLL) && !defined(WIN32) && !defined(WIN64)

void Server::BuryTheDead()
{
	UTIL_PTR_LIST_FOREACH(this->clients, Client, it)
	{
		if (!it->Connected() && ((it->send_buffer.length() == 0 && it->recv_buffer.length() == 0) || it->closed_time + 2 < std::time(0)))
		{
#if defined(WIN32) || defined(WIN64)
			closesocket(it->sock);
#else // defined(WIN32) || defined(WIN64)
			close(it->sock);
#endif // defined(WIN32) || defined(WIN64)
			this->clients.erase(it);
		}
	}
}
