#pragma once

#include <iostream>

#include <vector>
#include <mutex>

#include "Socket.h"
#include "../../FileTransferTCP-Server/FileTransferTCP-Server/FileHandler.h"
//#include "FileHandler.h"
#include "DataStreamer.h"

class ICLI {
public:
	virtual ~ICLI() {};
	virtual void run(const SOCKET& clientSocket, const FileHandler& fileHandler, const DataStreamer& dataStreamer) = 0;
};

class CLI: public ICLI {
	enum Commands;
	std::string pathToClientStorage;
	std::string filename;

	std::mutex sendMsgMtx;
	std::condition_variable sendMessage;

	std::mutex waitFileReceivedMtx;
	std::condition_variable allClientsReceivedFile;
	bool isFileReceivedByEveryone = false;

	std::mutex sendingFileMtx;
	bool isSendingFile = false;

public:
	CLI();
	CLI(const CLI&) = delete;
	CLI(CLI&&) = delete;

	void run(const SOCKET& clientSocket, const FileHandler& fileHandler, const DataStreamer& dataStreamer) override;

	void receiveMessages(SOCKET clientSocket, const DataStreamer& dataStreamer, const FileHandler& fileHandler);
};
