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
	SocketServer() {
		int iResult = 0;
		//Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0)
			throw MAKESTRING("WSAStartup failed with error: %d\n", iResult);
	}
	~SocketServer()
	{
		WSACleanup();
	}
	void Setup(std::string PORT)
	{
		port = PORT;
		funcdata.cont = true;
		funcdata.socket = (PVOID) this;
		if (Handle != NULL)
			CloseHandle(Handle);
		Handle = CreateThread(NULL, 0, SetupFunc, (PVOID)&funcdata, 0, &ThreadID);

	}
	void Stop() 
	{
		funcdata.cont = false;
	}
	void Continue()
	{
		funcdata.cont = true;
	}
	void UpdateList()
	{
		std::vector<size_t> toDelete;
		for (size_t i = 0;i<sockets.size();i++)
		{
			if (sockets[i].isClosed())
			{
				toDelete.push_back(i);
			}
		}
		for (auto &i : toDelete)
		{
			sockets.erase(sockets.cbegin() + i);
		}
	}
	void WriteToAll(cstring buffer, size_t bufflen)
	{
		this->UpdateList();
		for (auto &i : sockets)
		{
			try {
				i.Write(buffer, bufflen);
			}
			catch (cstring err)
			{
				std::cout << " Error : " << err << std::endl;
			}
			
			i.Shutdown();
		}
	}

	cstring getPort() const { return port.c_str(); }

	std::vector<SocketServerOneConnection> sockets;

private:
	DWORD ThreadID;
	WSADATA wsaData;
	HANDLE Handle;
	FUNCDATA funcdata;
	std::string port;

};

DWORD WINAPI SetupFunc(PVOID arg)
{
	HANDLE Handle = 0;
	DWORD sign = 0;
	DWORD ThreadID = 0;
	PFUNCDATA data = (PFUNCDATA)arg;
	SocketServer* server = (SocketServer*)data->socket;
	do {
		SocketServer* server = (SocketServer*)data->socket;
		server->sockets.push_back(SocketServerOneConnection(false));
		if (server->sockets.size() > 0)
			server->sockets[SSIZE - 1].Setup(server->getPort());
		if (server->sockets.size() > 0)
			sign = server->sockets[SSIZE - 1].Read<DWORD>();
		if (sign != SIGN || !data->cont)
		{
			if (server->sockets.size() > 0)
				server->sockets[SSIZE - 1].Shutdown();
			if (server->sockets.size() > 0)
				server->sockets.pop_back();
		}
		else
		{
#ifdef DEBUG
			std::cout << "New client connected" << std::endl;
			std::cout << "Their is " << server->sockets.size() << " client(s) connected" << std::endl;
#endif // DEBUG

		}		
		CloseHandle(Handle);
	} while (data->cont);
	return 0;
}