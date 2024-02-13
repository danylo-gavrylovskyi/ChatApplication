#pragma once

#include <iostream>

#include <vector>

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
public:
	CLI();
	CLI(const CLI&) = delete;
	CLI(CLI&&) = delete;

	void run(const SOCKET& clientSocket, const FileHandler& fileHandler, const DataStreamer& dataStreamer) override;
};
