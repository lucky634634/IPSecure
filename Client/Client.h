#pragma once
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <cryptlib.h>
#include <sha.h>
#include <hex.h>
#include <files.h>
#include <sstream>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "cryptlib.lib")

#define MAX_BUFFER_SIZE 1024

class Client
{
public:
    Client();
    virtual ~Client();
    void Run(const char* ip, int port);

private:
    bool InitWsa();
    void CollectError();
    bool CreateSocket();
    bool Connect(const char* ip, int port);
    bool Authenticate();
    void HandleResponse();
    std::string GetSHA1(const std::string& input);

private:
    WSADATA m_wsaData;
    SOCKET m_clientSocket;
    std::atomic_bool m_running;
    std::string m_username;
    std::string m_password;
};

