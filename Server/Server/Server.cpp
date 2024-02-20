#include "Server.h"

Server::Server(std::mutex& consoleMtx): consoleMtx(consoleMtx), dataStreamer(consoleMtx){}

int Server::start(Socket& serverSocket, const int port, const FileHandler& fileHandler)
{
	std::thread messageQueueThread([this]() {
		incomingMessageHandler();
		});

	while (true) {
		SOCKET clientSocket = serverSocket.acceptConnection();
		if (clientSocket == INVALID_SOCKET) {
			std::lock_guard<std::mutex> lock(consoleMtx);
			std::cerr << "Accept failed.\n";
			serverSocket.closeConnection();
			return -1;
		}

		{
			std::lock_guard<std::mutex> lock(consoleMtx);
			std::cout << "Client " << clientSocket << " connected.\n";
		}

		std::thread clientThread([this, clientSocket, &fileHandler]() {
									handleClient(clientSocket, fileHandler);
								});
		clientThread.detach();
	}	messageQueueThread.join();	serverSocket.closeConnection();
	return 0;
}

void Server::handleClient(const SOCKET& clientSocket, const FileHandler& fileHandler) {
	std::string username = dataStreamer.receiveMessage(clientSocket);
	User user{ username, clientSocket };

	int32_t roomId = dataStreamer.receiveInt(clientSocket);
	joinRoom(roomId, user);

	while (true)
	{
		std::string receivedMessage = dataStreamer.receiveMessage(clientSocket);

		if (receivedMessage == "") {
			getRoomById(roomId).removeClient(user);
			pushToQueue(Message{ "--- " + user.username + " left ---", user.clientSocket, roomId });
			closesocket(clientSocket);
			break;
		}
		else if (receivedMessage == "/quit") {
			getRoomById(roomId).removeClient(user);
			pushToQueue(Message{ "--- " + user.username + " left ---", user.clientSocket, roomId });
			roomId = dataStreamer.receiveInt(clientSocket);
			joinRoom(roomId, user);
		}
		else if (receivedMessage == "/file") {
			pushToQueue(Message{ receivedMessage, user.clientSocket, roomId });
			std::string filename = dataStreamer.receiveMessage(clientSocket);
			std::string pathToFile = SERVER_STORAGE + filename;
			dataStreamer.receiveChunkedDataToFile(clientSocket, pathToFile, fileHandler);
			Message sendFileMsg{ username + " wants to send " + move(filename) + " file which size is " + fileHandler.getFileSize(pathToFile) + ", do you want to receive ? (/receive or /reject)", clientSocket, roomId };
			pushToQueue(sendFileMsg);

			int toBeReceived = getNumOfClients(roomId) - 1;
			std::unique_lock<std::mutex> lock(allFilesReceivedMtx);
			fileReceived.wait(lock, [this, toBeReceived]() { return numOfReceivedFiles == toBeReceived; });
			dataStreamer.sendMessage(clientSocket, "/received");
			numOfReceivedFiles = 0;
		}
		else if (receivedMessage == "/receive") {
			std::string filename = dataStreamer.receiveMessage(clientSocket);
			dataStreamer.sendFileUsingChunks(clientSocket, SERVER_STORAGE + filename, 100000);
			incrementNumOfReceivedFiles(getNumOfClients(roomId) - 1);
		}
		else if (receivedMessage == "/reject") incrementNumOfReceivedFiles(getNumOfClients(roomId) - 1);
		else {
			std::string msgWithUsername = username + std::string(": ") + move(receivedMessage);
			Message msg{ move(msgWithUsername), clientSocket, roomId };
			pushToQueue(std::move(msg));
		}
	}

	{
		std::lock_guard<std::mutex> lock(consoleMtx);
		std::cout << "Client " << clientSocket << " disconnected.\n";
	}
	closesocket(clientSocket);
}

void Server::pushToQueue(const Message& msg) {
	std::lock_guard<std::mutex> lock(msgQueueMtx);
	messageQueue.push(msg);
	isNewMessage.notify_one();
}
void Server::broadcastMessage(const Message& msg) {
	std::lock_guard<std::mutex> lock(consoleMtx);
	for (const User& user : getRoomById(msg.roomId).getClients()) {
		if (user.clientSocket != msg.sender) {
			dataStreamer.sendMessage(user.clientSocket, msg.content);
		}
	}
}
void Server::incomingMessageHandler() {
	while (true) {
		std::unique_lock<std::mutex> lock(msgQueueMtx);
		isNewMessage.wait(lock, [this] { return !messageQueue.empty(); });
		while (!messageQueue.empty()) {
			Message message = messageQueue.front();
			messageQueue.pop();
			broadcastMessage(message);
		}
	}
}

Room& Server::getRoomById(const int roomId) {
	std::lock_guard<std::mutex> lock(roomMtx);
	auto it = std::find_if(this->rooms.begin(), this->rooms.end(), [roomId](const Room& room) {return room.getId() == roomId; });
	Room& room = *it;
	return room;
}
void Server::joinRoom(const int roomId, const User& user) {
	std::lock_guard<std::mutex> lock(roomMtx);
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

	pushToQueue(Message{ "--- " + user.username + " joined ---", user.clientSocket, roomId });
}
int Server::getNumOfClients(const int roomId) {
	return getRoomById(roomId).getClients().size();
}

void Server::incrementNumOfReceivedFiles(int toBeReceived) {
	std::unique_lock<std::mutex> lock(allFilesReceivedMtx);
	numOfReceivedFiles++;
	if (numOfReceivedFiles == toBeReceived) fileReceived.notify_one();
}
