#include "Server.h"

Server::Server()
{
	m_wsaData = { 0 };
	m_serverSocket = INVALID_SOCKET;
	m_clientSocket = INVALID_SOCKET;
}

Server::~Server()
{
	CloseSocket();
}

void Server::Run(int port)
{
	Load("users.txt");
	if (!InitWsa())
	{
		return;
	}

	std::cout << "Server initializing..." << std::endl;
	if (!CreateSocket())
	{
		return;
	}

	std::cout << "Server started on port " << port << "..." << std::endl;
	if (!BindSocket(port))
	{
		return;
	}



	m_running = true;
	std::cout << "Server listening..." << std::endl;
	if (!ListenSocket())
	{
		return;
	}
	std::cout << "Waiting for client..." << std::endl;
	if (!AcceptClient())
	{
		return;
	}
	if (!Authenticate())
	{
		std::cout << "Authentication failed." << std::endl;
		return;
	}
	std::cout << "Client connected." << std::endl;
	do
	{
		std::thread t = std::thread(&Server::HandleClient, this);
		std::string msg;
		std::cout << "Enter message: ";
		std::cin >> msg;
		int status = send(m_clientSocket, msg.c_str(), msg.size(), 0);
		if (msg == "exit")
		{
			m_running = false;
			closesocket(m_clientSocket);
		}
		t.join();
	} while (m_running && m_clientSocket != INVALID_SOCKET);
	closesocket(m_clientSocket);
}

bool Server::InitWsa()
{
	int status = WSAStartup(MAKEWORD(2, 2), &m_wsaData);
	if (status != 0)
	{
		return false;
	}
	return true;
}

void Server::CollectError()
{
	std::cerr << "Error: " << WSAGetLastError() << std::endl;
}

bool Server::CreateSocket()
{
	m_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (m_serverSocket == INVALID_SOCKET)
	{
		CollectError();
		return false;
	}
	return true;
}

bool Server::BindSocket(int port)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(m_serverSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		CollectError();
		return false;
	}
	return true;
}

bool Server::ListenSocket()
{
	int status = listen(m_serverSocket, SOMAXCONN);
	if (status == SOCKET_ERROR)
	{
		CollectError();
		return false;
	}
	return true;
}

bool Server::AcceptClient()
{
	if (m_clientSocket != INVALID_SOCKET)
	{
		closesocket(m_clientSocket);
		m_clientSocket = INVALID_SOCKET;
	}

	m_clientSocket = accept(m_serverSocket, NULL, NULL);
	if (m_clientSocket == INVALID_SOCKET)
	{
		CollectError();
		return false;
	}
	return true;
}

void Server::CleanupWsa()
{
	WSACleanup();
}

void Server::CloseSocket()
{
	if (m_clientSocket != INVALID_SOCKET)
	{
		closesocket(m_clientSocket);
		m_clientSocket = INVALID_SOCKET;
	}
	if (m_serverSocket != INVALID_SOCKET)
	{
		closesocket(m_serverSocket);
		m_serverSocket = INVALID_SOCKET;
	}
	CleanupWsa();
}

bool Server::Authenticate()
{
	char buffer[MAX_BUFFER_SIZE] = { 0 };
	int status = recv(m_clientSocket, buffer, MAX_BUFFER_SIZE, 0);
	if (status == SOCKET_ERROR)
	{
		CollectError();
		return false;
	}

	std::string msg(buffer);
	std::string username = msg.substr(0, msg.find(":"));
	std::string password = msg.substr(msg.find(":") + 1);

	if (m_users.find(username) != m_users.end() && m_users[username] == password)
	{
		std::cout << "Authentication successful." << std::endl;
		send(m_clientSocket, "OK", 2, 0);
		return true;
	}
	return false;
}

void Server::Load(const std::string& path)
{
	std::ifstream file(path);
	if (file.is_open())
	{
		std::string line;
		while (std::getline(file, line))
		{
			std::string username = line.substr(0, line.find(":"));
			std::string password = line.substr(line.find(":") + 1);
			m_users[username] = password;
		}
		file.close();
	}
}

void Server::HandleClient()
{

	char buffer[MAX_BUFFER_SIZE] = { 0 };
	int status = recv(m_clientSocket, buffer, MAX_BUFFER_SIZE, 0);
	if (status == SOCKET_ERROR)
	{
		return;
	}
	std::cout << "Client: " << buffer << std::endl;
	if (std::string(buffer) == "exit")
	{
		closesocket(m_clientSocket);
		m_clientSocket = INVALID_SOCKET;
		m_running = false;
	}
}

void Server::SendMessage(std::string msg)
{

}

bool Server::PerformKeyExchange()
{
	return true;
}
