#include "SocketServer.h"
#include <iostream>

#define DEFAULT_PORT "27015"


int main()
{
	SocketServer server;
	server.Setup(DEFAULT_PORT);
	std::cin.get();
	server.Stop();
	std::cout << "Stoping conection" << std::endl;
	std::cin.get();
	server.WriteToAll("Good bye friend", 16);
	std::cout << "Closing connection" << std::endl;
	std::cin.get();
	return 0;
}