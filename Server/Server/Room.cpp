#include "Room.h"

Room::Room(int roomId): id(roomId){}
Room::Room(Room&& other): id(other.id), clients(other.clients){}

int Room::getId() const
{
	return this->id;
}

std::vector<User>& Room::getClients()
{
	return this->clients;
}

void Room::addClient(const User& clientSocket)
{
	clients.push_back(clientSocket);
}

void Room::removeClient(const User& user) {
	auto it = std::find_if(clients.begin(), clients.end(), [user](const User& roomUser) {return roomUser.username == user.username; });
	if (it != clients.end()) {
		clients.erase(it);
	}
}
