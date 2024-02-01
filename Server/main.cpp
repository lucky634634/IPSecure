#include <iostream>
#include "Server.h"

int main()
{
	Server server;
	if (!server.InitializeWsa())
	{
		return 1;
	}
	if (!server.InitializeSocket())
	{
		return 2;
	}
	if (!server.Bind(8080))
	{
		return 3;
	}
	if (!server.Listen())
	{
		return 4;
	}
	if (!server.Accept())
	{
		return 5;
	}
	char buffer[1024] = { 0 };
	server.ReceiveData(buffer);
	std::cout << buffer << std::endl;
	system("pause");
	return 0;
}