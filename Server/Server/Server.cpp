#include "Server.h"

Server::Server(std::lock_guard<std::mutex>& locker): locker(locker){}

enum Server::Commands {
	EXIT,
	GET,
	LIST,
	PUT,
	DELETE_FILE,
	INFO
};

void Server::start(Socket& serverSocket, const int port, const FileHandler& fileHandler)
{
	DataStreamer dataStreamer;

	std::vector<std::thread> threads;	while (true) {
		SOCKET clientSocket = serverSocket.acceptConnection();
		threads.emplace_back(
			[this, clientSocket, &fileHandler, &dataStreamer]() {
				handleClient(clientSocket, fileHandler, dataStreamer);
			});
	}	serverSocket.closeConnection();
	for (auto& thread : threads) {
		thread.join();
	}
}

void Server::handleClient(const SOCKET& clientSocket, const FileHandler& fileHandler, const DataStreamer& dataStreamer) {
	const char* SUBFOLDER = dataStreamer.receiveChunkedData(clientSocket);
	const std::string PATH_TO_FOLDER = "C:/Meine/KSE/ClientServer/FileTransferTCP/server_storage/" + std::string(SUBFOLDER);
	if (fileHandler.doesDirectoryExist(SUBFOLDER) == -1) fileHandler.createDirectory(PATH_TO_FOLDER);

	while (true)
	{
		int cmd = 0;
		int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&cmd), sizeof(int), 0);
		if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
			locker;
			std::cerr << "Error while receiving command number." << std::endl;
			return;
		}

		if (cmd == EXIT) {
			break;
		}

		switch (cmd)
		{
		case GET: {
			const char* filename = dataStreamer.receiveChunkedData(clientSocket);
			std::string pathToFile = PATH_TO_FOLDER + "/" + std::string(filename);

			dataStreamer.sendFileUsingChunks(clientSocket ,move(pathToFile), 100000000);

			delete[] filename;
			break;
		}
		case LIST: {
			std::vector<char*> splittedPath = {};
			std::string listOfFiles = "";

			for (const auto& entry : std::filesystem::directory_iterator(PATH_TO_FOLDER)) {
				std::filesystem::path outfilename = entry.path();
				std::string outfilename_str = outfilename.string();
				char* filename = move(outfilename_str).data();

				char* partOfPath = strtok(filename, "\\");
				while (partOfPath != NULL) {
					partOfPath = strtok(NULL, "\\");
					splittedPath.push_back(partOfPath);
				}

				listOfFiles += splittedPath[splittedPath.size() - 2] + std::string(" ");

				splittedPath.clear();
			}

			dataStreamer.sendChunkedData(clientSocket ,move(listOfFiles).c_str(), 10);

			break;
		}
		case PUT: {
			const char* filename = dataStreamer.receiveChunkedData(clientSocket);
			std::string pathToFile = PATH_TO_FOLDER + "/" + std::string(filename);

			dataStreamer.receiveChunkedDataToFile(clientSocket, move(pathToFile), fileHandler);

			dataStreamer.sendChunkedData(clientSocket, "File was uploaded successfully.", 10);

			this->locker;
			std::cout << "File '" << filename << "' was created" << std::endl;

			delete[] filename;
			break;
		}
		case DELETE_FILE: {
			const char* filename = dataStreamer.receiveChunkedData(clientSocket);
			std::string pathToFile = PATH_TO_FOLDER + "/" + std::string(filename);

			if (fileHandler.deleteFile(move(pathToFile)) == 0) {
				this->locker;
				std::cout << "File '" << filename << "' was deleted from the server storage." << std::endl;
				dataStreamer.sendChunkedData(clientSocket, "File was successfully deleted from the server storage.", 10);
			}
			else {
				this->locker;
				std::cout << "Error occured while deleting '" << filename << "' from the server storage." << std::endl;
				dataStreamer.sendChunkedData(clientSocket, "Error while deleting file from the server storage.", 10);
			}

			delete[] filename;
			break;
		}
		case INFO: {
			const char* filename = dataStreamer.receiveChunkedData(clientSocket);
			std::string pathToFile = PATH_TO_FOLDER + "/" + std::string(filename);

			char* fileInfo = fileHandler.getFileInfo(move(pathToFile));
			dataStreamer.sendChunkedData(clientSocket, fileInfo, 10);

			delete[] fileInfo;
			delete[] filename;
			break;
		}
		default:
			break;
		}
	}

	delete[] SUBFOLDER;
	closesocket(clientSocket);
}
