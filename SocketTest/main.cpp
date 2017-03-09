#include "SocketServer.h"
#include <iostream>

#define DEFAULT_PORT "27015"


int main()
{
	SocketServer server;
	server.Setup(DEFAULT_PORT);
	std::cin.get();
	server.Stop();
	for (auto &i : server.sockets)
	{
		i.Write("Good bye friend", 16);
		i.Shutdown();
	}
	server.Stop();
	std::cout << "Closing connection" << std::endl;
	std::cin.get();
	return 0;
}