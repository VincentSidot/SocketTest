
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

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
	PVOID SocketClientClass;
	void(*function)(PVOID);
}DATA, *PDATA;


DWORD WINAPI myfunction(PVOID args);


class SocketClient
{
public:

	SocketClient(){}
	DWORD Setup(cstring ip, cstring port)
	{
		try {
			int iResult;

			// Initialize Winsock
			iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (iResult != 0)
				throw MAKESTRING("WSAStartup failed with error: %d\n", iResult);


			ZeroMemory(&hints, sizeof(hints));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;

			// Resolve the server address and port
			iResult = getaddrinfo(ip, port, &hints, &result);
			if (iResult != 0)
				throw MAKESTRING("getaddrinfo failed with error: %d\n", iResult);
			for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

				// Create a SOCKET for connecting to server
				ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
					ptr->ai_protocol);
				if (ConnectSocket == INVALID_SOCKET)
					throw MAKESTRING("socket failed with error: %ld\n", WSAGetLastError());

				// Connect to server.
				iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
				if (iResult == SOCKET_ERROR) {
					closesocket(ConnectSocket);
					ConnectSocket = INVALID_SOCKET;
					continue;
				}
				break;
			}
			freeaddrinfo(result);
			result = NULL;
			if (ConnectSocket == INVALID_SOCKET)
				throw MAKESTRING("Unable to connect to server!\n");
			return 0;
		}
		catch (cstring str)
		{
			printf("%s", str);
			SocketClient::~SocketClient();
			return (DWORD)str[0];
		}
	}
	void Shutdown() { SocketClient::~SocketClient(); }
	~SocketClient()
	{
		shutdown(ConnectSocket, SD_SEND);
		if (result != NULL)
			freeaddrinfo(result);
		if (ConnectSocket != INVALID_SOCKET)
			closesocket(ConnectSocket);
		closesocket(ConnectSocket);
		WSACleanup();
	}
	DWORD AsyncRead(PVOID var, PDWORD byteread, DWORD varsize, void(*function)(PVOID), PVOID arg)
	{
		PDATA data = (PDATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DATA));
		data->arg = arg;
		data->byteread = byteread;
		data->var = var;
		data->varsize = varsize;
		data->function = function;
		data->SocketClientClass = (PVOID) this;
		ThreadID.push_back(0);
		HandleID.push_back(CreateThread(NULL, 0, myfunction, (PVOID)data, 0, &ThreadID[ThreadID.size() - 1]));
		return ThreadID[ThreadID.size() - 1];
	}
	void WaitForAllThread()
	{
		WaitForMultipleObjects(ThreadID.size(), HandleID.data(), TRUE, INFINITE);
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
	SOCKET getClientSocket() const { return ConnectSocket; }
	template<typename type>
	type Read()
	{
		type temp;
		int iResult;
		do {
			iResult = recv(ConnectSocket, (char*)&temp, sizeof(temp), 0);
		} while (iResult == 0);

		return temp;
	}
	size_t Read(cstring buffer, size_t bufferlen)
	{
		int iResult;
		do {
			iResult = recv(ConnectSocket, (char*)buffer, bufferlen, 0);
		} while (iResult <= 0);
		return iResult;
	}
	template<typename type>
	int Write(type data)
	{
		int iSendResult = send(ConnectSocket, (char*)&data, sizeof(data), 0);
		if (iSendResult == SOCKET_ERROR)
			throw MAKESTRING("send failed with error: %d\n", WSAGetLastError());
		return iSendResult;
	}
	int Write(cstring buffer, size_t bufferlen)
	{
		int iSendResult = send(ConnectSocket, buffer, bufferlen, 0);
		if (iSendResult == SOCKET_ERROR)
			throw MAKESTRING("send failed with error: %d\n", WSAGetLastError());
		return iSendResult;
	}
private:

	std::vector<DWORD> ThreadID;
	std::vector<HANDLE> HandleID;
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
};

DWORD WINAPI myfunction(PVOID args)
{
	PDATA data = (PDATA)args;
	SocketClient* socket = (SocketClient*)data->SocketClientClass;
	do {
		*data->byteread = recv(socket->getClientSocket(), (char*)data->var, data->varsize, 0);
	} while (data->byteread == 0);
	data->function(data->arg);
	return 0;
}