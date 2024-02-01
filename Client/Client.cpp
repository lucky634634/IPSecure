#include "Client.h"

Client::Client()
{
	m_wsaData = { 0 };
	m_socket = INVALID_SOCKET;
	m_socketAddress = { 0 };
}

Client::~Client()
{
	closesocket(m_socket);
	WSACleanup();
}

bool Client::InitializeWsa()
{
	int result = WSAStartup(MAKEWORD(2, 2), &m_wsaData);
	if (result != NO_ERROR)
	{
		std::cerr << "WSAStartup failed: " << WSAGetLastError() << std::endl;
		return false;
	}
	return true;
}

bool Client::InitializeSocket()
{
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket == INVALID_SOCKET)
	{
		std::cerr << "socket failed: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return false;
	}
	return true;
}

bool Client::Connect(const char* ip, int port)
{
	m_socketAddress.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &m_socketAddress.sin_addr);
	m_socketAddress.sin_port = htons(port);
	int result = connect(m_socket, (SOCKADDR*)&m_socketAddress, sizeof(m_socketAddress));
	if (result == SOCKET_ERROR)
	{
		std::cerr << "connect failed: " << WSAGetLastError() << std::endl;
		return false;
	}
	return true;
}

bool Client::SendData(const char* data)
{
	int result = send(m_socket, data, strlen(data) + 1, 0);
	if (result == SOCKET_ERROR)
	{
		std::cerr << "send failed: " << WSAGetLastError() << std::endl;
		return false;
	}
	return true;
}

bool Client::ReceiveData(char* buffer)
{
	memset(buffer, 0, MAX_BUFFER_SIZE);
	int result = recv(m_socket, buffer, MAX_BUFFER_SIZE, 0);
	if (result == SOCKET_ERROR)
	{
		std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
		return false;
	}
	return true;
}
