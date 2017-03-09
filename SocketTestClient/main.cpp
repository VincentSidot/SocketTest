#include <iostream>
#include "SocketClient.h"

#define DEFAULT_PORT "27015"
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
	client.Write("Hello server :)",15);
	float temp = client.Read<DWORD>();
	cout << "Number recived : " << temp << endl;
	client.Shutdown();
	cout << "Closing connection" << endl;
	std::cin.get();

}