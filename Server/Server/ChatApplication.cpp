#include <iostream>
#include <WinSock2.h>

#include <mutex>

#include "Socket.h"
#include "Server.h"
#include "FileHandler.h"

// Linking the library needed for network communication
#pragma comment(lib, "ws2_32.lib")

int main()
{
	std::mutex consoleMtx;

	const int PORT = 12345;

	Socket serverSocket;
	serverSocket.startUp(PORT);

	FileHandler fileHandler;

	Server server(consoleMtx);
	server.start(serverSocket, PORT, fileHandler);

	return 0;
}