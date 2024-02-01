#include <iostream>
#include "Client.h"

int main()
{
	Client client;
	if (!client.InitializeWsa())
	{
		return 1;
	}
	if (!client.InitializeSocket())
	{
		return 2;
	}
	if (!client.Connect("127.0.0.1", 8080))
	{
		return 3;
	}
	client.SendData("Hello");
	system("pause");
	return 0;
}