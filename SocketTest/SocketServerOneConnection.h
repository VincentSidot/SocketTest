#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <vector>

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 1024

using cstring = const char*;

char tmp[DEFAULT_BUFLEN];
cstring MAKESTRING(cstring str, ...)
{
	ZeroMemory(tmp, DEFAULT_BUFLEN);
	va_list va;
	va_start(va, str);
	vsprintf_s(tmp, str, va);
	va_end(va);
	return tmp;
}

typedef struct data
{
	PVOID arg;
	PVOID var;
	PDWORD byteread = 0;
	DWORD varsize;
	PVOID SocketServerOneConnectionClass;
	void(*function)(PVOID);
}DATA, *PDATA;


DWORD WINAPI myfunction(PVOID args);



class SocketServerOneConnection
{
public:

	SocketServerOneConnection(){}
	DWORD Setup(cstring port)
	{
		try {
			int iResult = 0;
			//Initialize Winsock
			iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (iResult != 0)
				throw MAKESTRING("WSAStartup failed with error: %d\n", iResult);

			ZeroMemory(&hints, sizeof(hints));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
			hints.ai_flags = AI_PASSIVE;

			// Resolve the server address and port
			iResult = getaddrinfo(NULL, port, &hints, &result);
			if (iResult != 0)
				throw MAKESTRING("getaddrinfo failed with error: %d\n", iResult);

			// Create a SOCKET for connecting to server
			ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
			if (ListenSocket == INVALID_SOCKET)
				throw MAKESTRING("socket failed with error: %ld\n", WSAGetLastError());

			// Setup the TCP listening socket
			iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
			if (iResult == SOCKET_ERROR)
				throw MAKESTRING("bind failed with error: %d\n", WSAGetLastError());
			freeaddrinfo(result);
			result = NULL;

			iResult = listen(ListenSocket, SOMAXCONN);
			if (iResult == SOCKET_ERROR)
				throw MAKESTRING("listen failed with error: %d\n", WSAGetLastError());

			// Accept a client socket
			ClientSocket = accept(ListenSocket, NULL, NULL);
			if (ClientSocket == INVALID_SOCKET)
				throw MAKESTRING("accept failed with error: %d\n", WSAGetLastError());

			// No longer need server socket
			closesocket(ListenSocket);
			ListenSocket = NULL;
			return 0;
		}
		catch (cstring str)
		{
			printf("%s", str);
			SocketServerOneConnection::~SocketServerOneConnection();
			return (DWORD)str[0];
		}
	}
	void Shutdown() { SocketServerOneConnection::~SocketServerOneConnection();}
	~SocketServerOneConnection()
	{
		shutdown(ClientSocket, SD_SEND);
		if(result != NULL)
			freeaddrinfo(result);
		if (ListenSocket != NULL)
		closesocket(ListenSocket);
		closesocket(ClientSocket);
		WSACleanup();
	}
	template<typename type>
	type Read()
	{
		type temp;
		int iResult;
		do {
			iResult = recv(ClientSocket, (char*)&temp, sizeof(temp), 0);
		} while (iResult == 0);
		
		return temp;
	}
	DWORD AsyncRead(PVOID var,PDWORD byteread,DWORD varsize, void (*function)(PVOID), PVOID arg)
	{
		PDATA data = (PDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DATA));
		data->arg = arg;
		data->byteread = byteread;
		data->var = var;
		data->varsize = varsize;
		data->function = function;
		data->SocketServerOneConnectionClass = (PVOID) this;
		ThreadID.push_back(0);
		HandleID.push_back(CreateThread(NULL, 0, myfunction, (PVOID)data, 0, &ThreadID[ThreadID.size()-1]));
		return ThreadID[ThreadID.size() - 1];
	}
	void WaitForAllThread()
	{
		WaitForMultipleObjects(ThreadID.size(),HandleID.data(), TRUE, INFINITE);
		return;
	}
	void WaitForThread(DWORD _ThreadID)
	{
		int i = 0;
		for (; i < ThreadID.size(); i++)
			if (ThreadID[i] == _ThreadID) break;
		WaitForSingleObject(HandleID[i], INFINITE);
		return;
	}
	SOCKET getClientSocket() const { return ClientSocket; }
	size_t Read(cstring buffer, size_t bufferlen)
	{
		int iResult;
		do {
			iResult = recv(ClientSocket,(char*) buffer, bufferlen, 0);
		} while (iResult <= 0);
		return iResult;
	}
	template<typename type>
	int Write(type data)
	{
		int iSendResult = send(ClientSocket,(char*) &data, sizeof(data), 0);
		if (iSendResult == SOCKET_ERROR)
			throw MAKESTRING("send failed with error: %d\n", WSAGetLastError());
		return iSendResult;
	}
	int Write(cstring buffer, size_t bufferlen)
	{
		int iSendResult = send(ClientSocket, buffer, bufferlen, 0);
		if (iSendResult == SOCKET_ERROR)
			throw MAKESTRING("send failed with error: %d\n", WSAGetLastError());
		return iSendResult;
	}


private:

	std::vector<DWORD> ThreadID;
	std::vector<HANDLE> HandleID;
	struct addrinfo *result = NULL;
	struct addrinfo hints;
	WSADATA wsaData;
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

};

DWORD WINAPI myfunction(PVOID args)
{
	PDATA data = (PDATA)args;
	SocketServerOneConnection* socket = (SocketServerOneConnection*)data->SocketServerOneConnectionClass;
	do {
		*data->byteread = recv(socket->getClientSocket(), (char*)data->var, data->varsize, 0);
	} while (data->byteread == 0);
	data->function(data->arg);
	return 0;
}