#pragma once
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include "Cryptography.h"


#pragma comment(lib, "Ws2_32.lib")


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
    bool PerformKeyExchange();
    void HandleResponse();
    void SendMsg(std::string message);

private:
    WSADATA m_wsaData;
    SOCKET m_clientSocket;
    std::atomic_bool m_running;
    std::string m_username;
    std::string m_password;

    std::string m_key;
    std::string m_iv;
};

