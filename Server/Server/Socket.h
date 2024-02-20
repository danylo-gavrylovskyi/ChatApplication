#pragma once

#include <iostream>
#include <WinSock2.h>

#include <mutex>

class ISocket {
public:
	virtual ~ISocket() {}
	
	virtual int startUp(int port) = 0;
	virtual SOCKET& acceptConnection() = 0;
	virtual int closeConnection() = 0;
};

class Socket: public ISocket {
	SOCKET mainSocket;
	std::mutex& consoleMtx;
public:
	Socket(std::mutex& consoleMtx);
	Socket(const Socket&) = delete;
	Socket(Socket&&) = delete;

	int startUp(int port) override;
	SOCKET& acceptConnection() override;
	int closeConnection() override;

	const SOCKET& getSocket() const;
};
