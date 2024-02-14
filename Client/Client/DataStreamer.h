#pragma once

#include <WinSock2.h>

#include <vector>
#include <string>

#include "FileHandler.h"

class IDataStreamer {
public:
	virtual ~IDataStreamer() {}

	virtual std::string receiveMessage(const SOCKET& clientSocket) const = 0;
	virtual int receiveChunkedDataToFile(const SOCKET& clientSocket, const std::string& pathToFile, const FileHandler& fileHandler) const = 0;

	virtual int sendFileUsingChunks(const SOCKET& clientSocket, std::string&& pathToFile, int chunkSize) const = 0;
	virtual int sendMessage(const SOCKET& clientSocket, const std::string& message) const = 0;
	virtual int sendIntData(const SOCKET& clientSocket, int num) const = 0;
};

class DataStreamer : public IDataStreamer {
public:
	std::string receiveMessage(const SOCKET& clientSocket) const override;
	int receiveChunkedDataToFile(const SOCKET& clientSocket, const std::string& pathToFile, const FileHandler& fileHandler) const override;

	int sendFileUsingChunks(const SOCKET& clientSocket, std::string&& pathToFile, int chunkSize) const override;
	int sendMessage(const SOCKET& clientSocket, const std::string& message) const override;
	int sendIntData(const SOCKET& clientSocket, int num) const override;
};

