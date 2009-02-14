
#include "socket.hpp"

#ifdef __WIN32__

#include <winsock2.h>
bool ws_init = false;
WSADATA wsadata;

#endif // WIN32

IPAddress::IPAddress()
{
	this->address = 0;
}

IPAddress::IPAddress(std::string str_addr)
{
	unsigned int io1, io2, io3, io4;
	sscanf(str_addr.c_str(), "%u.%u.%u.%u", &io1, &io2, &io3, &io4);
	uint8_t o1 = io1, o2 = io2, o3 = io3, o4 = io4;
	this->address = o1 << 24 | o2 << 16 | o3 << 8 | o4;
}

IPAddress::IPAddress(const char *str_addr)
{
	unsigned int io1, io2, io3, io4;
	sscanf(str_addr, "%u.%u.%u.%u", &io1, &io2, &io3, &io4);
	uint8_t o1 = io1, o2 = io2, o3 = io3, o4 = io4;
	this->address = o1 << 24 | o2 << 16 | o3 << 8 | o4;
}

IPAddress::IPAddress(uint32_t addr)
{
	this->address = addr;
}

IPAddress::IPAddress(in_addr addr)
{
	this->address = ntohl(addr.S_un.S_addr);
}

IPAddress::IPAddress(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4)
{
	this->address = o1 << 24 | o2 << 16 | o3 << 8 | o4;
}

IPAddress &IPAddress::operator =(std::string str_addr)
{
	unsigned int io1, io2, io3, io4;
	sscanf(str_addr.c_str(), "%u.%u.%u.%u", &io1, &io2, &io3, &io4);
	uint8_t o1 = io1, o2 = io2, o3 = io3, o4 = io4;
	this->address = o1 << 24 | o2 << 16 | o3 << 8 | o4;
	return *this;
}

IPAddress &IPAddress::operator =(const char *str_addr)
{
	unsigned int io1, io2, io3, io4;
	sscanf(str_addr, "%u.%u.%u.%u", &io1, &io2, &io3, &io4);
	uint8_t o1 = io1, o2 = io2, o3 = io3, o4 = io4;
	this->address = o1 << 24 | o2 << 16 | o3 << 8 | o4;
	return *this;
}

IPAddress &IPAddress::operator =(uint32_t addr)
{
	this->address = addr;
	return *this;
}

IPAddress::operator uint32_t()
{
	return this->address;
}

IPAddress::operator in_addr()
{
	in_addr addr;
	addr.S_un.S_addr = htonl(this->address);
	return addr;
}

IPAddress::operator std::string()
{
	char buf[16];
	uint8_t o1, o2, o3, o4;
	o1 = (this->address & 0xFF000000) >> 24;
	o2 = (this->address & 0x00FF0000) >> 16;
	o3 = (this->address & 0x0000FF00) >> 8;
	o4 = this->address & 0x000000FF;
	sprintf(buf, "%u.%u.%u.%u", o1, o2, o3, o4);
	return std::string(buf);
}

Client::Client()
{
#ifdef WIN32
	if (!ws_init)
	{
		WSAStartup(0x0200, &wsadata);
		ws_init = true;
	}
#endif // WIN32
	this->connected = false;
}

Client::Client(SOCKET sock, sockaddr_in sin)
{
	this->connected = true;
	this->sock = sock;
	this->sin = sin;
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

bool Client::Connected()
{
	return this->connected;
}

bool Client::Close()
{
	if (this->connected)
	{
#ifdef WIN32
		return closesocket(this->sock) == 0;
#else // WIN32
		return close(this->sock) == 0;
#endif // WIN32
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

Client::~Client()
{
	if (this->connected)
	{
		this->Close();
	}
}
