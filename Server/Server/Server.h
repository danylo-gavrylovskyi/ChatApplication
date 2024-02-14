#pragma once

#include <filesystem>

#include <thread>
#include <vector>
#include <queue>

#include "Socket.h"
#include "FileHandler.h"
#include "DataStreamer.h"
#include "Room.h"
#include "User.h"

class IServer {
public:
	virtual ~IServer(){}

	virtual int start(Socket& serverSocket, const int port, const FileHandler& fileHandler) = 0;
	virtual void handleClient(const SOCKET& clientSocket, const FileHandler& fileHandler, const DataStreamer& dataStreamer) = 0;
};

class Server: public IServer {
	std::mutex& consoleMtx;
	std::mutex msgQueueMtx;
	std::vector<Room> rooms;
	std::queue<std::string> messageQueue;
	std::condition_variable isNewMessage;
public:
	Server(std::mutex& consoleMtx);
	Server(const Server&) = delete;
	Server(Server&&) = delete;

	int start(Socket& serverSocket, const int port, const FileHandler& fileHandler) override;
	void handleClient(const SOCKET& clientSocket, const FileHandler& fileHandler, const DataStreamer& dataStreamer) override;

	void pushToQueue(const std::string& msg);
	void broadcastMessage(const std::string& msg, const SOCKET clientSocket, Room& room, const DataStreamer& dataStreamer);
	void incomingMessageHandler(SOCKET clientSocket, const DataStreamer& dataStreamer, const int roomId);

	Room& getRoomById(const int roomId);
};
