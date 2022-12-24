/* socket.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifdef CLANG_MODULES_WORKAROUND
#include "platform.h"
#include "socket_impl.hpp"
#endif // CLANG_MODULES_WORKAROUND

#include "socket.hpp"
#include "console.hpp"
#include "util.hpp"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <stdexcept>
#include <string>
#include <vector>

#include "platform.h"

#include "socket_impl.hpp"

#ifdef WIN32
static WSADATA socket_wsadata;
#endif // WIN32

static char ErrorBuf[1024];

static std::size_t eoserv_strlcpy(char *dest, const char *src, std::size_t size)
{
	if (size > 0)
	{
		std::size_t src_size = std::strlen(src);

		if (src_size > size)
			size = src_size;

		std::memcpy(dest, src, size-1);
		dest[size-1] = '\0';
	}

	return size;
}

#ifdef WIN32
const char *OSErrorString()
{
	int error = GetLastError();

	if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, ErrorBuf, 1023, 0))
	{
		using namespace std;
		snprintf(ErrorBuf, 1024, "Unknown error %i", error);
	}
	else
	{
		eoserv_strlcpy(ErrorBuf, util::trim(ErrorBuf).c_str(), 1024);
	}

	return ErrorBuf;
}
#else // WIN32
// POSIX string.h for strerror
#include <string.h>
const char *OSErrorString()
{
	eoserv_strlcpy(ErrorBuf, strerror(errno), sizeof(ErrorBuf));
	return ErrorBuf;
}
#endif // WIN32

void Socket_Init::init()
{
#ifdef WIN32
	if (WSAStartup(MAKEWORD(2,0), &socket_wsadata) != 0)
	{
		throw Socket_InitFailed(OSErrorString());
	}
#endif // WIN32
}

static Socket_Init socket_init;

IPAddress::IPAddress()
{
	this->SetInt(0);
}

IPAddress::IPAddress(std::string str_addr)
{
	this->SetString(str_addr);
}

IPAddress::IPAddress(const char *str_addr)
{
	this->SetString(str_addr);
}

IPAddress::IPAddress(unsigned int addr)
{
	this->SetInt(addr);
}

IPAddress::IPAddress(unsigned char o1, unsigned char o2, unsigned char o3, unsigned char o4)
{
	this->SetOctets(o1, o2, o3, o4);
}

IPAddress IPAddress::Lookup(std::string host)
{
	addrinfo hints;
	addrinfo *ai = 0;
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

	if (ai)
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

IPAddress::operator std::string() const
{
	return this->GetString();
}

bool IPAddress::operator ==(const IPAddress &other) const
{
	return (this->address == other.address);
}

struct Client::impl_
{
	SOCKET sock;
	sockaddr_in sin;

	impl_(const SOCKET &sock = SOCKET(), const sockaddr_in &sin = sockaddr_in())
		: sock(sock)
		, sin(sin)
	{ }
};

Client::Client()
	: impl(new impl_(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
	, server(0)
	, connected(false)
	, connect_time(0)
	, recv_buffer_gpos(0)
	, recv_buffer_ppos(0)
	, recv_buffer_used(0)
	, send_buffer_gpos(0)
	, send_buffer_ppos(0)
	, send_buffer_used(0)
{ }

Client::Client(const IPAddress &addr, uint16_t port)
	: impl(new impl_(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
	, server(0)
	, connected(false)
	, connect_time(0)
	, recv_buffer_gpos(0)
	, recv_buffer_ppos(0)
	, recv_buffer_used(0)
	, send_buffer_gpos(0)
	, send_buffer_ppos(0)
	, send_buffer_used(0)
{
	this->Connect(addr, port);
}

Client::Client(Server *server)
	: impl(new impl_(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
	, server(server)
	, connected(false)
	, connect_time(0)
	, recv_buffer_gpos(0)
	, recv_buffer_ppos(0)
	, recv_buffer_used(0)
	, send_buffer_gpos(0)
	, send_buffer_ppos(0)
	, send_buffer_used(0)
{ }

Client::Client(const Socket &sock, Server *server)
	: impl(new impl_(sock.sock, sock.sin))
	, server(server)
	, connected(true)
	, connect_time(std::time(0))
	, recv_buffer_gpos(0)
	, recv_buffer_ppos(0)
	, recv_buffer_used(0)
	, send_buffer_gpos(0)
	, send_buffer_ppos(0)
	, send_buffer_used(0)
{ }

inline void assert_power_of_two(std::size_t size)
{
	while ((size & 1) == 0)
	{
		size >>= 1;
	}

	if (size > 1)
		throw std::runtime_error("Buffer size must be a power of two");
}

void Client::SetRecvBuffer(std::size_t size)
{
	assert_power_of_two(size);
	this->recv_buffer.resize(size);
}

void Client::SetSendBuffer(std::size_t size)
{
	assert_power_of_two(size);
	this->send_buffer.resize(size);
}

bool Client::Connect(const IPAddress &addr, uint16_t port)
{
	std::memset(&this->impl->sin, 0, sizeof(this->impl->sin));
	this->impl->sin.sin_family = AF_INET;
	this->impl->sin.sin_addr.s_addr = htonl(addr);
	this->impl->sin.sin_port = htons(port);

	if (connect(this->impl->sock, reinterpret_cast<sockaddr *>(&this->impl->sin), sizeof(this->impl->sin)) != 0)
	{
		return this->connected = false;
	}

	this->connect_time = std::time(0);

	return this->connected = true;
}

void Client::Bind(const IPAddress &addr, uint16_t port)
{
	sockaddr_in sin;
	uint16_t portn = htons(port);

#ifdef WIN32
	BOOL yes = 1;
	setsockopt(this->impl->sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(BOOL));
#else
	int yes = 1;
	setsockopt(this->impl->sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
#endif

	std::memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(addr);
	sin.sin_port = portn;

	if (bind(this->impl->sock, reinterpret_cast<sockaddr *>(&sin), sizeof(sin)) == SOCKET_ERROR)
	{
		throw Socket_BindFailed(OSErrorString());
	}
}

std::string Client::Recv(std::size_t length)
{
	length = std::min(length, this->recv_buffer_used);

	std::string ret(length, char());

	const std::size_t mask = this->recv_buffer.length() - 1;

	for (std::size_t i = 0; i < length; ++i)
	{
		this->recv_buffer_gpos = (this->recv_buffer_gpos + 1) & mask;
		ret[i] = this->recv_buffer[this->recv_buffer_gpos];
	}

	this->recv_buffer_used -= length;

	return ret;
}

void Client::Send(const std::string &data)
{
	if (data.length() > this->send_buffer.length() - this->send_buffer_used)
	{
		this->Close(true);
		return;
	}

	const std::size_t mask = this->send_buffer.length() - 1;

	for (std::size_t i = 0; i < data.length(); ++i)
	{
		this->send_buffer[this->send_buffer_ppos] = data[i];
		this->send_buffer_ppos = (this->send_buffer_ppos + 1) & mask;
	}

	this->send_buffer_used += data.length();
}

bool Client::DoRecv()
{
	char buf[8192];

	const int to_recv = std::min(this->recv_buffer.length() - this->recv_buffer_used, sizeof(buf));

	if (to_recv == 0)
		return false;

	const int recieved = recv(this->impl->sock, buf, to_recv, 0);

	if (recieved > 0)
	{
		const std::size_t mask = this->recv_buffer.length() - 1;

		for (int i = 0; i < recieved; ++i)
		{
			this->recv_buffer_ppos = (this->recv_buffer_ppos + 1) & mask;
			this->recv_buffer[this->recv_buffer_ppos] = buf[i];
		}

		this->recv_buffer_used += recieved;
	}
	else
	{
		return false;
	}

	return true;
}

bool Client::DoSend()
{
	char buf[8192];

	const std::size_t mask = this->send_buffer.length() - 1;
	const std::size_t gpos = this->send_buffer_gpos;

	std::size_t to_send;
	for (to_send = 0; to_send < std::min(this->send_buffer_used, sizeof(buf)); ++to_send)
	{
		buf[to_send] = this->send_buffer[this->send_buffer_gpos];
		this->send_buffer_gpos = (this->send_buffer_gpos + 1) & mask;
	}

	const int written = send(this->impl->sock, buf, to_send, 0);

	if (written < 0 || written == SOCKET_ERROR)
		return false;

	this->send_buffer_gpos = (gpos + written) & mask;
	this->send_buffer_used -= written;

	return true;
}

#if defined(SOCKET_POLL) && !defined(WIN32)
bool Client::Select(double timeout)
{
	pollfd fd;

	fd.fd = this->impl->sock;
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
			this->Close(true);
			return false;
		}

		if (fd.revents & POLLIN)
		{
			if (!this->DoRecv())
			{
				this->Close(true);
				return false;
			}
		}

		if (fd.revents & POLLOUT)
		{
			if (!this->DoSend())
			{
				this->Close(true);
				return false;
			}
		}

		return true;
	}

	return false;
}
#else // defined(SOCKET_POLL) && !defined(WIN32)
bool Client::Select(double timeout)
{
	fd_set read_fds, write_fds, except_fds;
	long tsecs = long(timeout);
	timeval timeout_val = {tsecs, long((timeout - double(tsecs))*1000000)};

	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);
	FD_ZERO(&except_fds);

	if (this->recv_buffer_used != this->recv_buffer.length())
	{
		FD_SET(this->impl->sock, &read_fds);
	}

	if (this->send_buffer_used > 0)
	{
		FD_SET(this->impl->sock, &write_fds);
	}

	FD_SET(this->impl->sock, &except_fds);

	int result = select(this->impl->sock+1, &read_fds, &write_fds, &except_fds, &timeout_val);
	if (result == -1)
	{
		throw Socket_SelectFailed(OSErrorString());
	}

	if (result > 0)
	{
		if (FD_ISSET(this->impl->sock, &except_fds))
		{
			this->Close(true);
			return false;
		}

		if (FD_ISSET(this->impl->sock, &read_fds))
		{
			if (!this->DoRecv())
			{
				this->Close(true);
				return false;
			}
		}

		if (FD_ISSET(this->impl->sock, &write_fds))
		{
			if (!this->DoSend())
			{
				this->Close(true);
				return false;
			}
		}

		return true;
	}

	return false;
}
#endif // defined(SOCKET_POLL) && !defined(WIN32)

bool Client::Connected() const
{
	return this->connected;
}

void Client::Close(bool force)
{
	if (this->Connected())
	{
		this->connected = false;
		this->closed_time = std::time(0);

		// This won't work properly for the first 2 seconds of 1 January 1970
		if (force)
		{
			this->closed_time = 0;
		}

		if (!this->server)
		{
#ifdef WIN32
			closesocket(this->impl->sock);
#else // WIN32
			close(this->impl->sock);
#endif // WIN32
		}
	}
}

void Client::FinishWriting()
{
	finished_writing = true;
}

IPAddress Client::GetRemoteAddr() const
{
	return IPAddress(ntohl(this->impl->sin.sin_addr.s_addr));
}

time_t Client::ConnectTime() const
{
	return this->connect_time;
}

Client::~Client()
{
	this->Close(true);
}

struct Server::impl_
{
	fd_set read_fds;
	fd_set write_fds;
	fd_set except_fds;
	SOCKET sock;

	impl_(const SOCKET &sock = INVALID_SOCKET)
		: sock(sock)
	{ }
};

Server::Server()
	: impl(new impl_(socket(AF_INET, SOCK_STREAM, 0)))
	, state(Created)
	, recv_buffer_max(32 * 1024)
	, send_buffer_max(32 * 1024)
	, maxconn(0)
{ }

Server::Server(const IPAddress &addr, uint16_t port)
	: impl(new impl_(socket(AF_INET, SOCK_STREAM, 0)))
	, state(Created)
	, recv_buffer_max(32 * 1024)
	, send_buffer_max(32 * 1024)
	, maxconn(0)
{
	this->Bind(addr, port);
}

void Server::Bind(const IPAddress &addr, uint16_t port)
{
	sockaddr_in sin;
	this->address = addr;
	this->port = port;
	this->portn = htons(port);

#ifdef WIN32
	BOOL yes = 1;
	setsockopt(this->impl->sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&yes), sizeof(BOOL));
#else
	int yes = 1;
	setsockopt(this->impl->sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
#endif

	std::memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(this->address);
	sin.sin_port = this->portn;

	if (bind(this->impl->sock, reinterpret_cast<sockaddr *>(&sin), sizeof(sin)) == SOCKET_ERROR)
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
		if (listen(this->impl->sock, backlog) != SOCKET_ERROR)
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
#ifdef WIN32
	unsigned long nonblocking;
#endif // WIN32

#ifdef WIN32
	nonblocking = 1;
	ioctlsocket(this->impl->sock, FIONBIO, &nonblocking);
#else // WIN32
	fcntl(this->impl->sock, F_SETFL, FNONBLOCK|FASYNC);
#endif // WIN32
	if ((newsock = accept(this->impl->sock, reinterpret_cast<sockaddr *>(&sin), &addrsize)) == INVALID_SOCKET)
	{
		return 0;
	}
#ifdef WIN32
	nonblocking = 0;
	ioctlsocket(this->impl->sock, FIONBIO, &nonblocking);
#else // WIN32
	fcntl(this->impl->sock, F_SETFL, 0);
#endif // WIN32

	// Close uninitialized connections to make room for new ones
	if (this->clients.size() >= this->maxconn)
	{
		for (auto it = this->clients.begin();
		 it != this->clients.end() && this->clients.size() >= this->maxconn;
		 ++it)
		{
			Client* client = *it;
			if (!client->accepted)
			{
				client->Close(true);
#ifdef WIN32
				closesocket(client->impl->sock);
#else // WIN32
				close(client->impl->sock);
#endif // WIN32
				delete client;
				it = this->clients.erase(it);
				if (it == this->clients.end())
					break;
			}
		}

		// If the server truly is full, fail the connection
		if (this->clients.size() >= this->maxconn)
		{
#ifdef WIN32
			closesocket(newsock);
#else // WIN32
			close(newsock);
#endif // WIN32
			return 0;
		}
	}

#if !defined(SOCKET_POLL) && !defined(WIN32)
	if (newsock >= FD_SETSIZE)
	{
		Console::Wrn("Client rejected due to file descriptor limits (%d / %d)", int(newsock), int(FD_SETSIZE) - 1);
		this->maxconn = std::min<unsigned>(this->maxconn, this->clients.size() - 1);
#ifdef WIN32
		closesocket(newsock);
#else // WIN32
		close(newsock);
#endif // WIN32
		return nullptr;
	}
#endif // !defined(SOCKET_POLL) && !defined(WIN32)

	newclient = this->ClientFactory(Socket(newsock, sin));
	newclient->SetRecvBuffer(this->recv_buffer_max);
	newclient->SetSendBuffer(this->send_buffer_max);

	this->clients.push_back(newclient);

	return newclient;
}

#if defined(SOCKET_POLL) && !defined(WIN32)
std::vector<Client *> *Server::Select(double timeout)
{
	static std::vector<Client *> selected;
	std::vector<pollfd> fds;
	int result;
	pollfd fd;

	fds.reserve(this->clients.size() + 1);

	fd.fd = this->impl->sock;
	fd.events = POLLERR;
	fds.push_back(fd);

	UTIL_FOREACH(this->clients, client)
	{
		fd.fd = client->impl->sock;

		fd.events = 0;

		if (client->recv_buffer_used != client->recv_buffer.length())
		{
			fd.events |= POLLIN;
		}

		if (client->send_buffer_used > 0)
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
		UTIL_FOREACH(this->clients, client)
		{
			++i;
			if (fds[i].revents & POLLERR || fds[i].revents & POLLHUP || fds[i].revents & POLLNVAL)
			{
				client->Close(true);
				continue;
			}

			if (fds[i].revents & POLLIN)
			{
				if (!client->DoRecv())
				{
					client->Close(true);
					continue;
				}
			}

			if (fds[i].revents & POLLOUT)
			{
				if (!client->DoSend())
				{
					client->Close(true);
					continue;
				}
			}
		}
	}

	UTIL_FOREACH(this->clients, client)
	{
		if (client->recv_buffer_used > 0 || client->NeedTick())
		{
			selected.push_back(client);
		}
	}

	return &selected;
}
#else // defined(SOCKET_POLL) && !defined(WIN32)
std::vector<Client *> *Server::Select(double timeout)
{
	long tsecs = long(timeout);
	timeval timeout_val = {tsecs, long((timeout - double(tsecs))*1000000)};
	static std::vector<Client *> selected;
	SOCKET nfds = this->impl->sock;
	int result;

	FD_ZERO(&this->impl->read_fds);
	FD_ZERO(&this->impl->write_fds);
	FD_ZERO(&this->impl->except_fds);

	UTIL_FOREACH(this->clients, client)
	{
		if (client->recv_buffer_used != client->recv_buffer.length())
		{
			FD_SET(client->impl->sock, &this->impl->read_fds);
		}

		if (client->send_buffer_used > 0)
		{
			FD_SET(client->impl->sock, &this->impl->write_fds);
		}

		FD_SET(client->impl->sock, &this->impl->except_fds);

		if (client->impl->sock > nfds)
		{
			nfds = client->impl->sock;
		}
	}

	FD_SET(this->impl->sock, &this->impl->except_fds);

	result = select(nfds+1, &this->impl->read_fds, &this->impl->write_fds, &this->impl->except_fds, &timeout_val);

	if (result == -1)
	{
		throw Socket_SelectFailed(OSErrorString());
	}

	if (result > 0)
	{
		if (FD_ISSET(this->impl->sock, &this->impl->except_fds))
		{
			throw Socket_Exception("There was an exception on the listening socket.");
		}

		UTIL_FOREACH(this->clients, client)
		{
			if (FD_ISSET(client->impl->sock, &this->impl->except_fds))
			{
				client->Close(true);
				continue;
			}

			if (FD_ISSET(client->impl->sock, &this->impl->read_fds))
			{
				if (!client->DoRecv())
				{
					client->Close(true);
					continue;
				}
			}

			if (FD_ISSET(client->impl->sock, &this->impl->write_fds))
			{
				if (!client->DoSend())
				{
					client->Close(true);
					continue;
				}
			}
		}
	}

	UTIL_FOREACH(this->clients, client)
	{
		if (client->recv_buffer_used > 0 || client->NeedTick())
		{
			selected.push_back(client);
		}

		if (client->send_buffer_used == 0 && client->finished_writing)
		{
#ifdef WIN32
			shutdown(client->impl->sock, SD_SEND);
#else
			shutdown(client->impl->sock, SHUT_WR);
#endif
		}
	}

	return &selected;
}
#endif // defined(SOCKET_POLL) && !defined(WIN32)

void Server::BuryTheDead()
{
	UTIL_IFOREACH(this->clients, it)
	{
		Client *client = *it;

		if (!client->Connected() && ((client->send_buffer.length() == 0 && client->recv_buffer.length() == 0) || client->closed_time + 2 < std::time(0)))
		{
#ifdef WIN32
			closesocket(client->impl->sock);
#else // WIN32
			close(client->impl->sock);
#endif // WIN32
			delete client;
			it = this->clients.erase(it);
			if (it == this->clients.end())
				break;
		}
	}
}

Server::~Server()
{
	UTIL_FOREACH(this->clients, client)
	{
#ifdef WIN32
		closesocket(client->impl->sock);
#else // WIN32
		close(client->impl->sock);
#endif // WIN32
		delete client;
	}

#ifdef WIN32
	closesocket(this->impl->sock);
#else // WIN32
	close(this->impl->sock);
#endif // WIN32

	delete this->impl;
}
