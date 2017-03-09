#include <iostream>
#include "SocketClient.h"

#define DEFAULT_PORT "27015"
char buffer[1024];
char bufflen = 1024;

using namespace std;

int main()
{
	SocketClient client;
	if (client.Setup("127.0.0.1",DEFAULT_PORT))
	{
		std::cout << "Error connecting server" << std::endl;
		cin.get();
		ExitProcess(-1);
	}

	client.Write<DWORD>(0xDEADBEEF);
	client.Read(buffer, bufflen);
	cout << "Server says : " << buffer << endl;
	std::cin.get();
	client.Shutdown();
	cout << "Closing connection" << endl;

}