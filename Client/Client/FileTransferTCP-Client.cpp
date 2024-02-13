#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>

#include "Socket.h"
#include "CLI.h"
#include "FileHandler.h"
#include "DataStreamer.h"

// Linking the library needed for network communication
#pragma comment(lib, "ws2_32.lib")

int main()
{
	const int PORT = 12345;
	PCWSTR const SERVER_IP = L"127.0.0.1";

	CLI cli;
	FileHandler fileHandler;
	DataStreamer dataStreamer;

	Socket clientSocket;
	clientSocket.connect(SERVER_IP, PORT);
	cli.run(clientSocket.getClientSocket(), fileHandler, dataStreamer);
	clientSocket.disconnect();

	return 0;
}