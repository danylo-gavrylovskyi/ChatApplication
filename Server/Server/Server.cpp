#include "Server.h"

Server::Server(std::lock_guard<std::mutex>& locker): locker(locker){}

int Server::start(Socket& serverSocket, const int port, const FileHandler& fileHandler)
{
	DataStreamer dataStreamer;

	while (true) {
		SOCKET clientSocket = serverSocket.acceptConnection();
		if (clientSocket == INVALID_SOCKET) {
			std::cerr << "Accept failed.\n";
			serverSocket.closeConnection();
			return -1;
		}

		this->locker;
		std::cout << "Client " << clientSocket << " connected.\n";

		std::thread clientThread([this, clientSocket, &fileHandler, &dataStreamer]() {
									handleClient(clientSocket, fileHandler, dataStreamer);
								});
		clientThread.detach();
	}	serverSocket.closeConnection();
	return 0;
}

void Server::handleClient(const SOCKET& clientSocket, const FileHandler& fileHandler, const DataStreamer& dataStreamer) {
	std::vector<char> username = dataStreamer.receiveChunkedData(clientSocket);
	std::cout << username.data();

	closesocket(clientSocket);
}
