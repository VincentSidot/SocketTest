#include "SocketServer.h"
#include <iostream>

#define DEFAULT_PORT "27015"


struct myDATA
{
	char str[DEFAULT_BUFLEN];
	DWORD read;
	PVOID socket;
	DWORD *ID;
};

void myFunc(PVOID arg)
{
	myDATA *data = (myDATA*)arg;
	data->str[data->read] = 0;
	SocketServer* socket = (SocketServer*) data->socket;
	std::cout << "Thread ID : " << *data->ID << std::endl;
	std::cout << "Message recived : " << data->str << std::endl;
	std::cout << "Bytes reads : " << data->read << std::endl;
	socket->Write<DWORD>(*data->ID);
	return;
}

int main()
{

	SocketServer server;
	if (server.Setup(DEFAULT_PORT))
	{
		std::cout << "Error creating server" << std::endl;
		std::cin.get();
		ExitProcess(-1);
	}
	DWORD threadID;
	myDATA data;
	data.ID = &threadID;
	data.socket = &server;
	threadID = server.AsyncRead((PVOID)data.str,&data.read,DEFAULT_BUFLEN, myFunc, (PVOID)&data);
	server.WaitForThread(threadID);
	server.Shutdown();
	std::cout << "Closing connection" << std::endl;
	std::cin.get();
	return 0;
}