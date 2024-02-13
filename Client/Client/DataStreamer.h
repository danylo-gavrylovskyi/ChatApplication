#pragma once

#include <WinSock2.h>

#include <vector>
#include <string>

#include "../../FileTransferTCP-Server/FileTransferTCP-Server/FileHandler.h"
//#include "FileHandler.h"

class IDataStreamer {
public:
	virtual ~IDataStreamer() {}

	virtual char* receiveChunkedData(const SOCKET& clientSocket) const = 0;
	virtual int receiveChunkedDataToFile(const SOCKET& clientSocket, const std::string& pathToFile, const FileHandler& fileHandler) const = 0;

	virtual int sendFileUsingChunks(const SOCKET& clientSocket, std::string&& pathToFile, int chunkSize) const = 0;
	virtual int sendChunkedData(const SOCKET& clientSocket, const char* data, int chunkSize) const = 0;
	virtual int sendIntData(const SOCKET& clientSocket, int num) const = 0;
};

class DataStreamer : public IDataStreamer {
public:
	char* receiveChunkedData(const SOCKET& clientSocket) const override;
	int receiveChunkedDataToFile(const SOCKET& clientSocket, const std::string& pathToFile, const FileHandler& fileHandler) const override;

	int sendFileUsingChunks(const SOCKET& clientSocket, std::string&& pathToFile, int chunkSize) const override;
	int sendChunkedData(const SOCKET& clientSocket, const char* data, int chunkSize) const override;
	int sendIntData(const SOCKET& clientSocket, int num) const override;
};

