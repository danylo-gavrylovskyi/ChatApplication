#include "Server.h"

Server::Server(std::mutex& consoleMtx): consoleMtx(consoleMtx){}

int Server::start(Socket& serverSocket, const int port, const FileHandler& fileHandler)
{
	DataStreamer dataStreamer(consoleMtx);

	std::thread messageQueueThread([this, &dataStreamer]() {
		incomingMessageHandler(dataStreamer);
		});

	while (true) {
		SOCKET clientSocket = serverSocket.acceptConnection();
		if (clientSocket == INVALID_SOCKET) {
			std::cerr << "Accept failed.\n";
			serverSocket.closeConnection();
			return -1;
		}

		{
			std::lock_guard<std::mutex> lock(consoleMtx);
			std::cout << "Client " << clientSocket << " connected.\n";
		}

		std::thread clientThread([this, clientSocket, &fileHandler, &dataStreamer]() {
									handleClient(clientSocket, fileHandler, dataStreamer);
								});
		clientThread.detach();
	}	messageQueueThread.join();	serverSocket.closeConnection();
	return 0;
}

void Server::handleClient(const SOCKET& clientSocket, const FileHandler& fileHandler, const DataStreamer& dataStreamer) {
	std::string username = dataStreamer.receiveMessage(clientSocket);
	User user{ username, clientSocket };

	int32_t roomId = 0;
	int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&roomId), sizeof(int32_t), 0);
	if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
		std::lock_guard<std::mutex> lock(consoleMtx);
		std::cerr << "Error while receiving room id." << std::endl;
		return;
	}

	auto it = std::find_if(this->rooms.begin(), this->rooms.end(), [roomId](const Room& room) {return room.getId() == roomId; });
	if (it != this->rooms.end()) {
		Room& room = *it;
		room.addClient(user);
	}
	else {
		Room room(roomId);
		room.addClient(user);
		this->rooms.push_back(std::move(room));
	}

	while (true)
	{
		std::string receivedMessage = dataStreamer.receiveMessage(clientSocket);

		if (receivedMessage == "") break;

		std::string msgWithUsername = username + std::string(": ") + std::move(receivedMessage);
		Message msg{ std::move(msgWithUsername), clientSocket, roomId };
		pushToQueue(std::move(msg));
	}

	closesocket(clientSocket);
}

void Server::pushToQueue(const Message& msg) {
	std::lock_guard<std::mutex> lock(msgQueueMtx);
	messageQueue.push(msg);
	isNewMessage.notify_one();
}
void Server::broadcastMessage(const Message& msg, const DataStreamer& dataStreamer) {
	std::lock_guard<std::mutex> lock(consoleMtx);
	for (const User& user : getRoomById(msg.roomId).getClients()) {
		if (user.clientSocket != msg.sender) {
			dataStreamer.sendMessage(user.clientSocket, msg.content);
		}
	}
}
void Server::incomingMessageHandler(const DataStreamer& dataStreamer) {
	while (true) {
		std::unique_lock<std::mutex> lock(msgQueueMtx);
		isNewMessage.wait(lock, [this] { return !messageQueue.empty(); });
		while (!messageQueue.empty()) {
			Message message = messageQueue.front();
			messageQueue.pop();
			broadcastMessage(message, dataStreamer);
		}
	}
}


Room& Server::getRoomById(const int roomId) {
	auto it = std::find_if(this->rooms.begin(), this->rooms.end(), [roomId](const Room& room) {return room.getId() == roomId; });
	Room& room = *it;
	return room;
}
