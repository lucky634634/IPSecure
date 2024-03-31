#include <iostream>
#include "Client.h"

int main()
{
	Client client;
	client.Run("127.0.0.1", 8080);
	system("pause");
	return 0;
}