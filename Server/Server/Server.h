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
#include "Message.h"

class IServer {
public:
	virtual ~IServer(){}

	virtual int start(Socket& serverSocket, const int port, const FileHandler& fileHandler) = 0;
	virtual void handleClient(const SOCKET& clientSocket, const FileHandler& fileHandler) = 0;
};

class Server: public IServer {
	std::mutex& consoleMtx;
	std::mutex msgQueueMtx;
	std::vector<Room> rooms;
	std::queue<Message> messageQueue;
	std::condition_variable isNewMessage;

	DataStreamer dataStreamer;
public:
	Server(std::mutex& consoleMtx);
	Server(const Server&) = delete;
	Server(Server&&) = delete;

	//server logic
	int start(Socket& serverSocket, const int port, const FileHandler& fileHandler) override;
	void handleClient(const SOCKET& clientSocket, const FileHandler& fileHandler) override;

	// message queue logic
	void pushToQueue(const Message& msg);
	void broadcastMessage(const Message& msg);
	void incomingMessageHandler();

	// room logic
	Room& getRoomById(const int roomId);
	void joinRoom(const int roomId, const User& user);
};
