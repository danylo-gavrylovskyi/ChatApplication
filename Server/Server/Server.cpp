#include "Server.h"

Server::Server(std::mutex& consoleMtx): consoleMtx(consoleMtx){}

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

		{
			std::lock_guard<std::mutex> lock(consoleMtx);
			std::cout << "Client " << clientSocket << " connected.\n";
		}

		std::thread clientThread([this, clientSocket, &fileHandler, &dataStreamer]() {
									handleClient(clientSocket, fileHandler, dataStreamer);
								});
		clientThread.detach();
	}	serverSocket.closeConnection();
	return 0;
}

void Server::handleClient(const SOCKET& clientSocket, const FileHandler& fileHandler, const DataStreamer& dataStreamer) {
	std::vector<char> username = dataStreamer.receiveChunkedData(clientSocket);
	User user{ std::string(username.data()), clientSocket };

	int32_t roomId = 0;
	int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&roomId), sizeof(int32_t), 0);
	if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
		std::lock_guard<std::mutex> lock(consoleMtx);
		std::cerr << "Error while receiving room id." << std::endl;
		return;
	}

	std::thread messageQueueThread([this, clientSocket, &dataStreamer, roomId]() {
		incomingMessageHandler(clientSocket, dataStreamer, roomId);
		});

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
		std::vector<char> msg = dataStreamer.receiveChunkedData(clientSocket);
		pushToQueue(std::string(msg.data()));
	}

	closesocket(clientSocket);
}

void Server::pushToQueue(const std::string& msg) {
	std::lock_guard<std::mutex> lock(consoleMtx);
	messageQueue.push(msg);
	isNewMessage.notify_one();
}
void Server::broadcastMessage(const std::string& msg, const SOCKET clientSocket, Room& room, const DataStreamer& dataStreamer) {
	std::lock_guard<std::mutex> lock(consoleMtx);
	for (const User& user : room.getClients()) {
		if (user.clientSocket != clientSocket) {
			dataStreamer.sendChunkedData(user.clientSocket, msg.c_str(), 10);
		}
	}
}
void Server::incomingMessageHandler(SOCKET clientSocket, const DataStreamer& dataStreamer, const int roomId) {
	while (true) {
		std::unique_lock<std::mutex> lock(msgQueueMtx);
		isNewMessage.wait(lock, [this] { return !messageQueue.empty(); });
		while (!messageQueue.empty()) {
			std::string message = messageQueue.front();
			messageQueue.pop();
			broadcastMessage(message, clientSocket, getRoomById(roomId), dataStreamer);
		}
	}
}

Room& Server::getRoomById(const int roomId) {
	auto it = std::find_if(this->rooms.begin(), this->rooms.end(), [roomId](const Room& room) {return room.getId() == roomId; });
	Room& room = *it;
	return room;
}
