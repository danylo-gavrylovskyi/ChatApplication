#pragma once

#include <vector>

#include "User.h"

class Room {
	int id;
	std::vector<User> clients;
public:
	Room(int roomId);
	Room(const Room& other) = delete;
	Room(Room&& other);

	int getId() const;

	std::vector<User>& getClients();
	void addClient(const User& clientSocket);
	void removeClient(const User& user);
};
