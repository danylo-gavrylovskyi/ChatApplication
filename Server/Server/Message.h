#pragma once

#include <string>

#include <WinSock2.h>

struct Message {
	std::string content;
	SOCKET sender;
	int roomId;
};
