#define DEBUG


#include "SocketServerOneConnection.h"
#include <vector>
#include <iostream>
#include <Windows.h>


#define DEFAULT_BUFLEN 1024

using cstring = const char*;

#define SSIZE server->sockets.size()
#define SIGN 0xDEADBEEF

typedef struct FuncData
{
	PVOID socket;
	bool cont;
}FUNCDATA,*PFUNCDATA;

DWORD WINAPI SetupFunc(PVOID arg);

class SocketServer
{
public:
	SocketServer() {}
	void Setup(std::string PORT)
	{
		port = PORT;
		funcdata.cont = true;
		funcdata.socket = (PVOID) this;
		Handle = CreateThread(NULL, 0, SetupFunc, (PVOID)&funcdata, 0, &ThreadID);

	}
	void Stop() 
	{
		funcdata.cont = false;
		WaitForSingleObject(Handle, INFINITE);
		CloseHandle(Handle);
	}
	cstring getPort() const { return port.c_str(); }

	std::vector<SocketServerOneConnection> sockets;
private:
	DWORD ThreadID;
	HANDLE Handle;
	FUNCDATA funcdata;
	std::string port;

};

DWORD WINAPI SetupFunc(PVOID arg)
{
	PFUNCDATA data = (PFUNCDATA)arg;
	SocketServer* server = (SocketServer*)data->socket;
	do {
		server->sockets.push_back(SocketServerOneConnection());
		if (server->sockets[SSIZE - 1].Setup(server->getPort()) != 0)
			server->sockets.pop_back();
		DWORD sign = server->sockets[SSIZE - 1].Read<DWORD>();
		if (sign != SIGN)
		{
			server->sockets[SSIZE - 1].Shutdown();
			server->sockets.pop_back();
		}
		else
		{
#ifdef DEBUG
			std::cout << "New client connected" << std::endl;
			std::cout << "Their is " << server->sockets.size() << " client(s) connected" << std::endl;
#endif // DEBUG

		}
	} while (data->cont);
	return 0;
}