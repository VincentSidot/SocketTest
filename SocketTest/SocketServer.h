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

typedef struct StupData
{
	PVOID socket;
	PDWORD ret;
}STUPDATA,*PSTUPDATA;

DWORD WINAPI SetupFunc(PVOID arg);
DWORD WINAPI Stup(PVOID arg);

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
	HANDLE Handle = 0;
	DWORD ThreadID = 0;
	DWORD ret;
	STUPDATA StupData;
	StupData.ret = &ret;
	PFUNCDATA data = (PFUNCDATA)arg;
	StupData.socket = data->socket;
	SocketServer* server = (SocketServer*)data->socket;
	do {
		Handle = CreateThread(NULL, 0, Stup, (PVOID)&StupData, 0, &ThreadID);
		if (data->cont)
		{
			WaitForSingleObject(Handle, 1000);
			CloseHandle(Handle);
		}
		else
		{
			server->sockets.pop_back();
			CloseHandle(Handle);
		}
	} while (data->cont);
	return 0;
}
DWORD WINAPI Stup(PVOID arg)
{
	PSTUPDATA data = (PSTUPDATA)arg;
	SocketServer* server = (SocketServer*)data->socket;
	server->sockets.push_back(SocketServerOneConnection());
	server->sockets[SSIZE - 1].Setup(server->getPort());
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
	return 0;
}