#pragma once
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define MAX_BUFFER_SIZE 1024

class Client
{
public:
    Client();
    virtual ~Client();
    bool InitializeWsa();
    bool InitializeSocket();
    bool Connect(const char* ip, int port);
    bool SendData(const char* data);
    bool ReceiveData(char* buffer);
    void CloseConnection();

private:
    WSADATA m_wsaData;
    SOCKET m_socket;
    SOCKADDR_IN m_socketAddress;
};

