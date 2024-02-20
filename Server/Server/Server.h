#pragma once

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
	std::mutex roomMtx;
	std::mutex allFilesReceivedMtx;

	std::vector<Room> rooms;

	std::queue<Message> messageQueue;
	std::condition_variable isNewMessage;
	std::condition_variable fileReceived;

	int numOfReceivedFiles = 0;

	const std::string SERVER_STORAGE = "C:/Meine/KSE/ClientServer/ChatApplication/server_storage/";

	const DataStreamer dataStreamer;
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
	int getNumOfClients(const int roomId);

	//other
	void incrementNumOfReceivedFiles(int toBeReceived);
};
