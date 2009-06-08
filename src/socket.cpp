
#include <cstdio>
#include <ctime>

#include "socket.hpp"

#include "util.hpp"

char ErrorBuf[1024];

#if defined(WIN32) || defined(WIN64)
bool ws_init = false;
WSADATA wsadata;

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
	return strerror_r(errno, ErrorBuf, 1024);
}
#endif // defined(WIN32) || defined(WIN64)

IPAddress::IPAddress()
{
	Socket_WSAStartup();
	this->address = 0;
}

IPAddress::IPAddress(std::string str_addr)
{
	Socket_WSAStartup();
	unsigned int io1, io2, io3, io4;
	std::sscanf(str_addr.c_str(), "%u.%u.%u.%u", &io1, &io2, &io3, &io4);
	unsigned char o1 = io1, o2 = io2, o3 = io3, o4 = io4;
	this->address = o1 << 24 | o2 << 16 | o3 << 8 | o4;
}

IPAddress::IPAddress(const char *str_addr)
{
	Socket_WSAStartup();
	unsigned int io1, io2, io3, io4;
	std::sscanf(str_addr, "%u.%u.%u.%u", &io1, &io2, &io3, &io4);
	unsigned char o1 = io1, o2 = io2, o3 = io3, o4 = io4;
	this->address = o1 << 24 | o2 << 16 | o3 << 8 | o4;
}

IPAddress::IPAddress(unsigned int addr)
{
	Socket_WSAStartup();
	this->address = addr;
}

IPAddress::IPAddress(in_addr addr)
{
	Socket_WSAStartup();
	this->address = ntohl(addr.s_addr);
}

IPAddress::IPAddress(unsigned char o1, unsigned char o2, unsigned char o3, unsigned char o4)
{
	Socket_WSAStartup();
	this->address = o1 << 24 | o2 << 16 | o3 << 8 | o4;
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

IPAddress &IPAddress::operator =(std::string str_addr)
{
	unsigned int io1, io2, io3, io4;
	std::sscanf(str_addr.c_str(), "%u.%u.%u.%u", &io1, &io2, &io3, &io4);
	unsigned char o1 = io1, o2 = io2, o3 = io3, o4 = io4;
	this->address = o1 << 24 | o2 << 16 | o3 << 8 | o4;
	return *this;
}

IPAddress &IPAddress::operator =(const char *str_addr)
{
	unsigned int io1, io2, io3, io4;
	std::sscanf(str_addr, "%u.%u.%u.%u", &io1, &io2, &io3, &io4);
	unsigned char o1 = io1, o2 = io2, o3 = io3, o4 = io4;
	this->address = o1 << 24 | o2 << 16 | o3 << 8 | o4;
	return *this;
}

IPAddress &IPAddress::operator =(unsigned int addr)
{
	this->address = addr;
	return *this;
}

IPAddress::operator unsigned int()
{
	return this->address;
}

IPAddress::operator in_addr()
{
	in_addr addr;
	addr.s_addr = htonl(this->address);
	return addr;
}

IPAddress::operator std::string()
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

bool IPAddress::operator ==(const IPAddress &other)
{
	return (this->address == other.address);
}

Client::Client()
{
	this->connected = false;
	this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	this->send_buffer_max = static_cast<unsigned int>(-1);
	this->recv_buffer_max = static_cast<unsigned int>(-1);
}

Client::Client(IPAddress addr, uint16_t port)
{
	Socket_WSAStartup();
	this->connected = false;
	this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	this->send_buffer_max = static_cast<unsigned int>(-1);
	this->recv_buffer_max = static_cast<unsigned int>(-1);

	this->Connect(addr, port);
}

Client::Client(void *server)
{
	Socket_WSAStartup();
	this->connected = false;
	this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	this->server = server;
}

Client::Client(SOCKET sock, sockaddr_in sin, void *server)
{
	Socket_WSAStartup();
	this->connected = true;
	this->sock = sock;
	this->sin = sin;
	this->server = server;
}

bool Client::Connect(IPAddress addr, uint16_t port)
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

void Client::Bind(IPAddress addr, uint16_t port)
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

#ifdef SOCKET_POLL
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
#else // SOCKET_POLL
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
#endif // SOCKET_POLL

bool Client::Connected()
{
	return this->connected;
}

bool Client::Close()
{
	if (this->connected)
	{
		this->connected = false;
#if defined(WIN32) || defined(WIN64)
		return closesocket(this->sock) == 0;
#else // defined(WIN32) || defined(WIN64)
		return close(this->sock) == 0;
#endif // defined(WIN32) || defined(WIN64)
	}
	else
	{
		return false;
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
