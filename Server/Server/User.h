#pragma once

#include <string>
#include <vector>

#include <WinSock2.h>

struct User {
	std::string username;
	SOCKET clientSocket;
};
