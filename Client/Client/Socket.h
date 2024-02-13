#pragma once

#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>

#include <vector>

#include "../../FileTransferTCP-Server/FileTransferTCP-Server/FileHandler.h"

class ISocket {
public:
	virtual ~ISocket(){}

	virtual int connect(PCWSTR IP, int port) = 0;
	virtual int disconnect() = 0;
};

class Socket: public ISocket {
	SOCKET clientSocket;
public:
	Socket();
	Socket(const Socket&) = delete;
	Socket(Socket&&) = delete;

	int connect(PCWSTR IP, int port) override;
	int disconnect() override;

	const SOCKET& getClientSocket() const;
};
