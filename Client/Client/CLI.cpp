#include "CLI.h"

CLI::CLI(){}

void CLI::run(const SOCKET& clientSocket, const FileHandler& fileHandler, const DataStreamer& dataStreamer)
{
	std::string username;
	std::cout << "Enter username: ";
	std::cin >> username;
	dataStreamer.sendMessage(clientSocket, username);

	int roomId;
	std::cout << "Enter room id you want to join: ";
	std::cin >> roomId;
	dataStreamer.sendIntData(clientSocket, roomId);
	
	std::thread receiveThread([this, clientSocket, &dataStreamer]() {
		receiveMessages(clientSocket, dataStreamer);
		});

	std::string message;
	while (true)
	{
		std::getline(std::cin >> std::ws, message);
		dataStreamer.sendMessage(clientSocket, message);
		if (message == "/quit") {
			std::cout << "Enter room id you want to join: ";
			std::cin >> roomId;
			dataStreamer.sendIntData(clientSocket, roomId);
		}
	}
}

void CLI::receiveMessages(SOCKET clientSocket, const DataStreamer& dataStreamer) {
	while (true)
	{
		std::string msg = dataStreamer.receiveMessage(clientSocket);
		std::cout << msg << std::endl;
	}
}

