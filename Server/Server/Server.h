#pragma once

#include <filesystem>

#include <thread>
#include <vector>

#include "Socket.h"
#include "FileHandler.h"
#include "DataStreamer.h"
#include "Room.h"

class IServer {
public:
	virtual ~IServer(){}

	virtual int start(Socket& serverSocket, const int port, const FileHandler& fileHandler) = 0;
	virtual void handleClient(const SOCKET& clientSocket, const FileHandler& fileHandler, const DataStreamer& dataStreamer) = 0;
};

class Server: public IServer {
	std::lock_guard<std::mutex>& locker;
	std::vector<Room> rooms;
public:
	Server(std::lock_guard<std::mutex> &locker);
	Server(const Server&) = delete;
	Server(Server&&) = delete;

	int start(Socket& serverSocket, const int port, const FileHandler& fileHandler) override;
	void handleClient(const SOCKET& clientSocket, const FileHandler& fileHandler, const DataStreamer& dataStreamer) override;
};
