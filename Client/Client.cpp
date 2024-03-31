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

void Client::Run(const char* ip, int port)
{
	std::cout << "Enter username: ";
	std::cin >> m_username;
	std::cout << "Enter password: ";
	std::cin.ignore(MAXINT, '\n');
	std::cin >> m_password;
	std::cout << GetSHA1(m_password) << std::endl;
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
	if (!Connect(ip, port))
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
	m_running = true;
	do
	{
		std::thread t(&Client::HandleResponse, this);
		std::string msg;
		std::cout << "Enter message: ";
		std::cin >> msg;
		send(m_clientSocket, msg.c_str(), msg.size(), 0);
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
	std::string msg = m_username + ":" + m_password;
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

void Client::HandleResponse()
{
	char buffer[MAX_BUFFER_SIZE] = { 0 };
	int status = recv(m_clientSocket, buffer, MAX_BUFFER_SIZE, 0);
	if (status == SOCKET_ERROR)
	{
		return;
	}
	std::cout << "Server: " << buffer << std::endl;
	if (strcmp(buffer, "exit") == 0)
	{
		m_running = false;
	}
}

std::string Client::GetSHA1(const std::string& input)
{
	using namespace CryptoPP;
	std::string output;
	SHA1 hash;
	hash.Update((const byte*)input.data(), input.size());
	output.resize(hash.DigestSize());
	hash.Final((byte*)&output[0]);
	std::stringstream ss;
	HexEncoder encoder(new FileSink(ss));
	StringSource strsrc(output, true, new Redirector(encoder));
	ss >> output;

	return output;
}
