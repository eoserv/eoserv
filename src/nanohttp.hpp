
#include "socket.hpp"

/**
 * Super simple HTTP client
 */
class HTTP
{
	private:
		Client *client;
		std::string response;
		int status;
		bool done;

	public:
		HTTP(std::string host, unsigned short port, std::string path);

		static HTTP *RequestURL(std::string url);

		void Tick(int timeout);

		bool Done();
		int StatusCode();
		std::string Response();

		static std::string URLEncode(std::string);

		~HTTP();
};
