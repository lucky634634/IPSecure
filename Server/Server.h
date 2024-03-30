#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <thread>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <bcrypt.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "bcrypt.lib")

#define MAX_BUFFER_SIZE 1024

class Server
{
public:
	Server();
	virtual ~Server();
	void Run(int port = 8080);

private:
	bool InitWsa();
	void CollectError();
	bool CreateSocket();
	bool BindSocket(int port = 8080);
	bool ListenSocket();
	bool AcceptClient();
	void CleanupWsa();
	void CloseSocket();
	bool Authenticate();
	void Load(const std::string& path);
	void HandleClient();
	void SendMessage(std::string msg);
	bool PerformKeyExchange();

private:
	WSADATA m_wsaData;

	SOCKET m_serverSocket;
	SOCKET m_clientSocket;

	std::vector<char> m_sharedKey;

	std::map<std::string, std::string> m_users;
	std::thread m_thread;
};

