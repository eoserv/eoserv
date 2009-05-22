
#include "nanohttp.hpp"

#include <string>
#include <cstdio>

#include "socket.hpp"
#include "util.hpp"

HTTP::HTTP(std::string host, unsigned short port, std::string path, IPAddress outgoing)
{
	std::string request;

	this->done = false;

	request = "GET " + path + " HTTP/1.0\r\n"
	"Host: " + host + "\r\n"
	"User-Agent: EOSERV\r\n"
	"Accept: text/plain\r\n"
	"Connection: close\r\n"
	"\r\n";

	client = new Client;
	if (static_cast<unsigned int>(outgoing) != 0)
	{
		client->Bind(outgoing, util::rand(49152, 65535));
	}
	client->Connect(IPAddress::Lookup(host), port);

	this->done = false;

	client->Send(request);
}

HTTP *HTTP::RequestURL(std::string url, IPAddress outgoing)
{
	std::size_t loc, loc2;

	loc = url.find("//");
	if (loc == std::string::npos)
	{
		return 0;
	}

	if (url.substr(0, loc) != "http:")
	{
		return 0;
	}

	bool hasport = true;
	loc2 = url.find_first_of(':', loc+2);
	if (loc2 == std::string::npos)
	{
		hasport = false;
		loc2 = url.find_first_of('/', loc+2);
		if (loc2 == std::string::npos)
		{
			loc2 = url.length()-1;
		}
	}

	std::string host = url.substr(loc+2, loc2-loc-2);

	std::string sport;
	if (hasport)
	{
		loc = loc2;
		loc2 = url.find_first_of('/', loc);
		sport = url.substr(loc+1, loc2-loc-1);
	}
	else
	{
		sport = "80";
	}

	unsigned short port = util::to_int(sport);

	std::string path = url.substr(loc2);

	return new HTTP(host, port, path, outgoing);
}

void HTTP::Tick(int timeout)
{
	this->client->Tick(timeout);

	this->response += this->client->Recv(32767);

	if (!this->client->Connected())
	{
		if (this->response.length() > 12)
		{
			this->status = util::to_int(this->response.substr(9,3));
		}
		std::size_t startcontent = this->response.find("\r\n\r\n");
		if (startcontent == std::string::npos)
		{
			this->response = "";
		}
		else
		{
			this->response = this->response.substr(startcontent+4);
		}
		this->done = true;
	}
}

bool HTTP::Done()
{
	return this->done;
}

int HTTP::StatusCode()
{
	return this->status;
}

std::string HTTP::Response()
{
	return this->response;
}

std::string HTTP::URLEncode(std::string raw)
{
	std::string encoded;
	char buf[3];

	for (size_t i = 0; i < raw.length(); ++i)
	{
		char c = raw[i];

		if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
		{
			encoded += c;
		}
		else
		{
			snprintf(buf, 3, "%02x", c);
			encoded += std::string("%") + std::string(buf, 2);
		}
	}

	return encoded;
}

HTTP::~HTTP()
{
	delete this->client;
}
