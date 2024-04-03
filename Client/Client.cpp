#include "Client.h"


Client::Client()
{
	m_wsaData = { 0 };
	m_clientSocket = INVALID_SOCKET;;
}

Client::~Client()
{
	closesocket(m_clientSocket);
	WSACleanup();
}

void Client::Run(int port)
{
	std::string ip = "127.0.0.1";
	std::cout << "Enter server IP: ";
	std::getline(std::cin, ip);
	std::cout << "Enter username: ";
	std::cin >> m_username;
	std::cout << "Enter password: ";
	std::cin.ignore(MAXINT, '\n');
	std::cin >> m_password;
	std::cout << "Client is running" << std::endl;
	if (!InitWsa())
	{
		return;
	}
	std::cout << "Createing socket" << std::endl;
	if (!CreateSocket())
	{
		return;
	}
	std::cout << "Connecting to server" << std::endl;
	if (!Connect(ip.c_str(), port))
	{
		return;
	}

	std::cout << "Authenticating" << std::endl;
	if (!Authenticate())
	{
		std::cout << "Authentication failed" << std::endl;
		return;
	}
	std::cout << "Authentication successful" << std::endl;
	std::cout << "Performing key exchange" << std::endl;
	if (!PerformKeyExchange())
	{
		std::cout << "Key exchange failed" << std::endl;
		return;
	}
	std::cout << "Key exchange successful" << std::endl;
	m_running = true;
	do
	{
		std::thread t(&Client::HandleResponse, this);
		std::string msg;
		std::cout << "Enter message: ";
		std::cin >> msg;
		SendMsg(msg);
		if (msg == "exit")
		{
			m_running = false;
		}
		t.join();
	} while (m_running);

}

bool Client::InitWsa()
{
	int status = WSAStartup(MAKEWORD(2, 2), &m_wsaData);
	if (status != 0)
	{
		return false;
	}
	return true;
}

void Client::CollectError()
{
	std::cerr << "Error: " << WSAGetLastError() << std::endl;
}

bool Client::CreateSocket()
{
	m_clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (m_clientSocket == INVALID_SOCKET)
	{
		CollectError();
		return false;
	}
	return true;
}

bool Client::Connect(const char* ip, int port)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &addr.sin_addr);

	int status = connect(m_clientSocket, (sockaddr*)&addr, sizeof(addr));
	if (status == SOCKET_ERROR)
	{
		CollectError();
		return false;
	}
	return true;
}

bool Client::Authenticate()
{
	if (m_username.empty() || m_password.empty())
	{
		return false;
	}

	char buffer[MAX_BUFFER_SIZE] = { 0 };
	std::string msg = m_username + ":" + GetSHA1(m_password);
	int status = send(m_clientSocket, msg.c_str(), msg.length(), 0);
	if (status == SOCKET_ERROR)
	{
		CollectError();
		return false;
	}

	status = recv(m_clientSocket, buffer, MAX_BUFFER_SIZE, 0);
	if (status == SOCKET_ERROR)
	{
		CollectError();
		return false;
	}
	if (strcmp(buffer, "OK") != 0)
	{
		return false;
	}

	return true;
}

bool Client::PerformKeyExchange()
{
	char buffer[MAX_BUFFER_SIZE] = { 0 };
	int status = recv(m_clientSocket, buffer, MAX_BUFFER_SIZE, 0);
	if (status == SOCKET_ERROR)
	{
		CollectError();
		return false;
	}
	std::string temp = buffer;
	m_key = temp.substr(0, temp.find("\r\n"));
	m_iv = temp.substr(temp.find("\r\n") + 2);
	return true;
}

void Client::HandleResponse()
{
	char buffer[MAX_BUFFER_SIZE] = { 0 };
	int status = recv(m_clientSocket, buffer, MAX_BUFFER_SIZE, 0);
	if (status == SOCKET_ERROR)
	{
		return;
	}
	std::string decrypted = DecryptAES(std::string(buffer), m_key, m_iv);
	std::cout << "Server: " << decrypted << std::endl;
	if (decrypted == "exit")
	{
		m_running = false;
	}
}

void Client::SendMsg(std::string message)
{
	std::string encrypted = EncryptAES(message, m_key, m_iv);
	send (m_clientSocket, encrypted.c_str(), encrypted.length(), 0);
}

