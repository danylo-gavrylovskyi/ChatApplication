#pragma once

#include <string>

#include <WinSock2.h>

struct User {
	std::string username;
	SOCKET clientSocket;
};
