#include "Server.h"

Server::Server()
{
	m_wsaData = { 0 };
	m_serverSocket = INVALID_SOCKET;
	m_clientSocket = INVALID_SOCKET;
}

Server::~Server()
{
	CloseSocket();
}

void Server::Run(int port)
{
	Load("users.txt");
	if (!InitWsa())
	{
		return;
	}

	if (!CreateSocket())
	{
		return;
	}

	if (!BindSocket(port))
	{
		return;
	}

	int choice = 0;
	std::cout << "1. Send Message" << std::endl;
	std::cout << "0. Exit" << std::endl;
	std::cout << "Enter your choice: " << std::endl;

	switch (choice)
	{
	case 0:
		std::terminate();
		break;
	case 1:
		break;
	default:
		break;
	}
}

bool Server::InitWsa()
{
	int status = WSAStartup(MAKEWORD(2, 2), &m_wsaData);
	if (status != 0)
	{
		return false;
	}
	return true;
}

void Server::CollectError()
{
	std::cerr << "Error: " << WSAGetLastError() << std::endl;
}

bool Server::CreateSocket()
{
	m_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (m_serverSocket == INVALID_SOCKET)
	{
		CollectError();
		return false;
	}
	return true;
}

bool Server::BindSocket(int port)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(m_serverSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		CollectError();
		return false;
	}
	return true;
}

bool Server::ListenSocket()
{
	int status = listen(m_serverSocket, 1);
	if (status == SOCKET_ERROR)
	{
		CollectError();
		return false;
	}
	return true;
}

bool Server::AcceptClient()
{
	if (m_clientSocket != INVALID_SOCKET)
	{
		closesocket(m_clientSocket);
		m_clientSocket = INVALID_SOCKET;
	}

	sockaddr_in addr;
	int addr_size = sizeof(addr);

	m_clientSocket = accept(m_serverSocket, (sockaddr*)&addr, &addr_size);
	if (m_clientSocket == INVALID_SOCKET)
	{
		CollectError();
		return false;
	}
	return true;
}

void Server::CleanupWsa()
{
	WSACleanup();
}

void Server::CloseSocket()
{
	if (m_clientSocket != INVALID_SOCKET)
	{
		closesocket(m_clientSocket);
		m_clientSocket = INVALID_SOCKET;
	}
	if (m_serverSocket != INVALID_SOCKET)
	{
		closesocket(m_serverSocket);
		m_serverSocket = INVALID_SOCKET;
	}
	CleanupWsa();
}

bool Server::Authenticate()
{
	char buffer[MAX_BUFFER_SIZE] = { 0 };
	int status = recv(m_clientSocket, buffer, MAX_BUFFER_SIZE, 0);
	if (status == SOCKET_ERROR)
	{
		CollectError();
		return false;
	}

	std::string msg(buffer);
	std::string username = msg.substr(msg.find("Authenticate ") + 14, msg.find(":"));
	std::string password = msg.substr(msg.find(":") + 1);

	if (m_users.find(username) != m_users.end() && m_users[username] == password)
	{
		return true;
	}
	return false;
}

void Server::Load(const std::string& path)
{
	std::ifstream file(path);
	if (file.is_open())
	{
		std::string line;
		while (std::getline(file, line))
		{
			std::string username = line.substr(0, line.find(":"));
			std::string password = line.substr(line.find(":") + 1);
			m_users[username] = password;
		}
		file.close();
	}
}

void Server::HandleClient()
{
	if (!ListenSocket())
	{
		return;
	}

	if (!AcceptClient())
	{
		return;
	}

	if (!Authenticate())
	{
		return;
	}

	if (!PerformKeyExchange())
	{
		return;
	}

	char buffer[MAX_BUFFER_SIZE] = { 0 };
	int status = recv(m_clientSocket, buffer, MAX_BUFFER_SIZE, 0);
	if (status == SOCKET_ERROR)
	{
		CollectError();
		return;
	}
	std::cout << "Client: " << std::string(buffer, buffer + status) << std::endl;


}

void Server::SendMessage(std::string msg)
{

}

bool Server::PerformKeyExchange()
{
	BCRYPT_ALG_HANDLE hDhAlg;
	BCRYPT_KEY_HANDLE hPrivateKey, hPublicKey;
	DWORD dwDataLen;
	PBYTE pbPrivateKeyBlob = NULL, pbPublicKeyBlob = NULL;
	std::vector<char> serverPublicKey;

	// Open a handle to the Diffie-Hellman algorithm provider.
	if (!BCryptOpenAlgorithmProvider(&hDhAlg, BCRYPT_DH_ALGORITHM, NULL, 0))
	{
		std::cerr << "BCryptOpenAlgorithmProvider failed." << std::endl;
		return false;
	}

	// Generate a new key pair.
	if (!BCryptGenerateKeyPair(hDhAlg, &hPrivateKey, 128, 0))
	{
		std::cerr << "BCryptGenerateKeyPair failed." << std::endl;
		BCryptCloseAlgorithmProvider(hDhAlg, 0);
		return false;
	}

	// Calculate the size of the public key blob.
	if (!BCryptExportKey(hPrivateKey, NULL, BCRYPT_DH_PUBLIC_BLOB, NULL, 0, &dwDataLen, 0))
	{
		std::cerr << "BCryptExportKey (1) failed." << std::endl;
		BCryptDestroyKey(hPrivateKey);
		BCryptCloseAlgorithmProvider(hDhAlg, 0);
		return false;
	}

	pbPublicKeyBlob = new BYTE[dwDataLen];
	if (pbPublicKeyBlob == NULL)
	{
		std::cerr << "Memory allocation failed." << std::endl;
		BCryptDestroyKey(hPrivateKey);
		BCryptCloseAlgorithmProvider(hDhAlg, 0);
		return false;
	}

	// Export the public key.
	if (!BCryptExportKey(hPrivateKey, NULL, BCRYPT_DH_PUBLIC_BLOB, pbPublicKeyBlob, dwDataLen, &dwDataLen, 0))
	{
		std::cerr << "BCryptExportKey (2) failed." << std::endl;
		delete[] pbPublicKeyBlob;
		BCryptDestroyKey(hPrivateKey);
		BCryptCloseAlgorithmProvider(hDhAlg, 0);
		return false;
	}

	// Send the public key to the server.
	if (send(m_clientSocket, reinterpret_cast<char*>(pbPublicKeyBlob), dwDataLen, 0) == SOCKET_ERROR)
	{
		std::cerr << "Failed to send public key to server." << std::endl;
		delete[] pbPublicKeyBlob;
		BCryptDestroyKey(hPrivateKey);
		BCryptCloseAlgorithmProvider(hDhAlg, 0);
		return false;
	}

	// Receive the server's public key.
	serverPublicKey.resize(dwDataLen);
	int bytesReceived = recv(m_clientSocket, serverPublicKey.data(), dwDataLen, 0);
	if (bytesReceived == SOCKET_ERROR)
	{
		std::cerr << "Failed to receive server's public key." << std::endl;
		delete[] pbPublicKeyBlob;
		BCryptDestroyKey(hPrivateKey);
		BCryptCloseAlgorithmProvider(hDhAlg, 0);
		return false;
	}

	// Import the server's public key.
	if (!BCryptImportKeyPair(hDhAlg, NULL, BCRYPT_DH_PUBLIC_BLOB, &hPublicKey, reinterpret_cast<PBYTE>(serverPublicKey.data()), serverPublicKey.size(), 0))
	{
		std::cerr << "BCryptImportKeyPair failed." << std::endl;
		delete[] pbPublicKeyBlob;
		BCryptDestroyKey(hPrivateKey);
		BCryptCloseAlgorithmProvider(hDhAlg, 0);
		return false;
	}

	std::vector<BYTE> sharedKey(128);

	// Derive the shared secret.
	if (!BCryptSecretAgreement(hPrivateKey, hPublicKey, reinterpret_cast<BCRYPT_SECRET_HANDLE*>(&sharedKey[0]), 0))
	{
		std::cerr << "BCryptSecretAgreement failed." << std::endl;
		delete[] pbPublicKeyBlob;
		BCryptDestroyKey(hPrivateKey);
		BCryptDestroyKey(hPublicKey);
		BCryptCloseAlgorithmProvider(hDhAlg, 0);
		return false;
	}

	// Store the shared secret.
	m_sharedKey.resize(sharedKey.size());
	memcpy(&m_sharedKey[0], &sharedKey[0], sharedKey.size());

	// Clean up.
	delete[] pbPublicKeyBlob;
	BCryptDestroyKey(hPrivateKey);
	BCryptDestroyKey(hPublicKey);
	BCryptCloseAlgorithmProvider(hDhAlg, 0);

	return true;
}
