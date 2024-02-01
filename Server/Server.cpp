#include "Server.h"

Server::Server()
{
	m_wsaData = { 0 };
	m_port = 0;
	m_socket = INVALID_SOCKET;
	m_socketAddress = { 0 };
	m_clientSocket = INVALID_SOCKET;
	m_clientSocketAddress = { 0 };
}

Server::~Server()
{}

bool Server::InitializeWsa()
{
	int result = WSAStartup(MAKEWORD(2, 2), &m_wsaData);
	if (result != NO_ERROR)
	{
		std::cerr << "WSAStartup failed: " << WSAGetLastError() << std::endl;
		return false;
	}
	return true;
}

bool Server::InitializeSocket()
{
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket == INVALID_SOCKET)
	{
		std::cerr << "socket failed: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return false;
	}
	return true;
}

bool Server::Bind(int port)
{
	m_port = port;
	m_socketAddress.sin_family = AF_INET;
	m_socketAddress.sin_port = htons(m_port);
	m_socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	int result = bind(m_socket, (SOCKADDR*)&m_socketAddress, sizeof(m_socketAddress));
	if (result == SOCKET_ERROR)
	{
		std::cerr << "bind failed: " << WSAGetLastError() << std::endl;
		result = closesocket(m_socket);
		if (result == SOCKET_ERROR)
		{
			std::cerr << "closesocket failed: " << WSAGetLastError() << std::endl;
		}
		WSACleanup();
		return false;
	}
	return true;
}

bool Server::Listen()
{
	int result = listen(m_socket, 1);
	if (result == SOCKET_ERROR)
	{
		std::cerr << "listen failed: " << WSAGetLastError() << std::endl;
		result = closesocket(m_socket);
		if (result == SOCKET_ERROR)
		{
			std::cerr << "closesocket failed: " << WSAGetLastError() << std::endl;
		}
		WSACleanup();
		return false;
	}
	return true;
}

bool Server::Accept()
{
	int size = sizeof(m_clientSocketAddress);
	m_clientSocket = accept(m_socket, (SOCKADDR*)&m_clientSocketAddress, &size);
	if (m_clientSocket == INVALID_SOCKET)
	{
		std::cerr << "accept failed: " << WSAGetLastError() << std::endl;
		return false;
	}
	return true;
}

bool Server::SendData(const char* buffer)
{
	int result = send(m_clientSocket, buffer, strlen(buffer), 0);
	if (result == SOCKET_ERROR)
	{
		std::cerr << "send failed: " << WSAGetLastError() << std::endl;
		return false;
	}
	return true;
}

bool Server::ReceiveData(char* buffer)
{
	int result = recv(m_clientSocket, buffer, MAX_BUFFER_SIZE, 0);
	if (result == SOCKET_ERROR)
	{
		std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
		return false;
	}
	return true;
}
