#include "CLI.h"

CLI::CLI(){}

void CLI::run(const SOCKET& clientSocket, const FileHandler& fileHandler, const DataStreamer& dataStreamer)
{
	std::string username;
	std::cout << "Enter username: ";
	std::cin >> username;
	dataStreamer.sendMessage(clientSocket, username);

	int roomId;
	std::cout << "Enter room id you want to join: ";
	std::cin >> roomId;
	dataStreamer.sendIntData(clientSocket, roomId);

	std::cout << "Enter folder where you want to store received files: ";
	std::cin >> pathToClientStorage;
	
	std::thread receiveThread([this, clientSocket, &dataStreamer, &fileHandler]() {
		receiveMessages(clientSocket, dataStreamer, fileHandler);
		});

	std::string message;
	while (true)
	{
		std::getline(std::cin >> std::ws, message);
		
		if (message == "/receive" && !isSendingFile) {
			std::cout << "You have nothing to receive.\n";
			continue;
		}
		else if (isSendingFile && (message != "/receive" && message != "/reject")) {
			std::cout << "You must receive or reject the file.\n";
			continue;
		}

		dataStreamer.sendMessage(clientSocket, message);

		if (message == "/quit") {
			std::cout << "Enter room id you want to join: ";
			std::cin >> roomId;
			dataStreamer.sendIntData(clientSocket, roomId);
		}
		else if (message == "/file") {
			std::string pathToFile;
			std::cout << "Enter path to file: ";
			std::cin >> pathToFile;

			std::filesystem::path path(pathToFile);
			std::string filename = path.filename().string();

			dataStreamer.sendMessage(clientSocket, filename);
			dataStreamer.sendFileUsingChunks(clientSocket, move(pathToFile), 100000);
			std::cout << "File sent.\n";

			std::unique_lock<std::mutex> lock(waitFileReceivedMtx);
			allClientsReceivedFile.wait(lock, [this]() {
				return isFileReceivedByEveryone;
				});

			std::cout << "All room members received file.\n";
			isFileReceivedByEveryone = false;
		}
		else if (message == "/receive") {
			dataStreamer.sendMessage(clientSocket, filename);
			dataStreamer.receiveChunkedDataToFile(clientSocket, pathToClientStorage + "/" + filename, fileHandler);
			std::cout << "File received.\n";

			std::lock_guard<std::mutex> lock(sendingFileMtx);
			isSendingFile = false;
		}
		else if (message == "/reject") {
			std::lock_guard<std::mutex> lock(sendingFileMtx);
			isSendingFile = false;
		}
	}

	receiveThread.join();
	closesocket(clientSocket);
}

void CLI::receiveMessages(SOCKET clientSocket, const DataStreamer& dataStreamer, const FileHandler& fileHandler) {
	while (true)
	{
		std::lock_guard<std::mutex> lock(sendingFileMtx);
		if (!isSendingFile) {
			std::string msg = dataStreamer.receiveMessage(clientSocket);
			if (msg == "/file") {
				msg = dataStreamer.receiveMessage(clientSocket);
				std::cout << msg << std::endl;

				char* filename = strtok((char*)msg.c_str(), " ");
				for (int i = 0; i < 4; i++) { filename = strtok(NULL, " "); }
				this->filename = filename;

				isSendingFile = true;
			}
			else if (msg == "/received") {
				std::unique_lock<std::mutex> lock(waitFileReceivedMtx);
				isFileReceivedByEveryone = true;
				allClientsReceivedFile.notify_one();
			}
			else {
				std::cout << msg << std::endl;
			}
		}
	}
}

