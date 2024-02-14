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
