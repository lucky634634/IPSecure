#include "Client.h"

Client::Client()
{
    m_wsaData = { 0 };
    m_clientSocket = INVALID_SOCKET;;
}

Client::~Client()
{}

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
    std::string msg = "AUTHENTICATE " + m_username + ":" + m_password;
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
