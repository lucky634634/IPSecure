#pragma once
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define MAX_BUFFER_SIZE 1024

class Server
{
public:
	Server();
	virtual ~Server();
	bool InitializeWsa();
	bool InitializeSocket();
	bool Bind(int port);
	bool Listen();
	bool Accept();
	bool SendData(const char* buffer);
	bool ReceiveData(char* buffer);

private:
	WSAData m_wsaData;
	int m_port;
	SOCKET m_socket;
	SOCKADDR_IN m_socketAddress;

	SOCKET m_clientSocket;
	SOCKADDR_IN m_clientSocketAddress;
};

