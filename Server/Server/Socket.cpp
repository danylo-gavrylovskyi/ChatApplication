#include "Socket.h"

Socket::Socket(std::mutex& consoleMtx): consoleMtx(consoleMtx){}

int Socket::startUp(int port) {
	// Initialize Winsock
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "WSAStartup failed" << std::endl;
		return -1;
	}

	// Server configuration
	this->mainSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->mainSocket == INVALID_SOCKET)
	{
		std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return -1;
	}
	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(port);

	// Bind the socket
	if (bind(this->mainSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
	{
		std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
		this->closeConnection();
		return -1;
	}

	// Listen for incoming connections
	if (listen(this->mainSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
		this->closeConnection();
		return -1;
	}

	std::cout << "Server listening on port " << port << std::endl;

	return 0;
}
SOCKET& Socket::acceptConnection() {
	SOCKET clientSocket = accept(this->mainSocket, nullptr, nullptr);
	if (clientSocket == INVALID_SOCKET)
	{
		std::lock_guard<std::mutex> lock(consoleMtx);
		std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
	}

	return clientSocket;
}
int Socket::closeConnection() {
	// Clean up
	closesocket(this->mainSocket);
	WSACleanup();
	return 0;
}

const SOCKET& Socket::getSocket() const {
	return this->mainSocket;
}
