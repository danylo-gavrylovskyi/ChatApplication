#include "DataStreamer.h"

std::vector<char> DataStreamer::receiveChunkedData(const SOCKET& clientSocket) const {
	int32_t totalSize = 0;
	int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&totalSize), sizeof(int), 0);
	if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
		std::cerr << "Error in receiving total size." << std::endl;
	}

	int32_t chunkSize = 0;
	bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&chunkSize), sizeof(int), 0);
	if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
		std::cerr << "Error in receiving chunk size." << std::endl;
	}

	std::vector<char> assembledData(totalSize + 1);
	int totalReceived = 0;

	while (totalReceived < totalSize) {
		std::vector<char> buffer(chunkSize);
		int bytesReceived = recv(clientSocket, buffer.data(), sizeof(buffer), 0);

		if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
			std::cerr << "Error in receiving chunked data." << std::endl;
			break;
		}

		std::copy(buffer.begin(), buffer.begin() + bytesReceived, assembledData.begin() + totalReceived);
		totalReceived += bytesReceived;
	}

	assembledData[totalReceived] = '\0';

	return assembledData;
}
int DataStreamer::receiveChunkedDataToFile(const SOCKET& clientSocket, const std::string& pathToFile, const FileHandler& fileHandler) const {
	long long totalSize = 0;
	int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&totalSize), sizeof(long long), 0);
	if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
		std::cerr << "Error in receiving total size." << std::endl;
		return -1;
	}

	long long totalReceived = 0;
	while (totalReceived < totalSize) {
		long long chunkSize = 0;
		int chunkBytesReceived = recv(clientSocket, reinterpret_cast<char*>(&chunkSize), sizeof(long long), 0);
		if (chunkBytesReceived == SOCKET_ERROR || chunkBytesReceived == 0) {
			std::cerr << "Error in receiving chunk size." << std::endl;
			return -1;
		}

		std::cout << "Received chunk of size: " << chunkSize << std::endl;

		std::vector<char> buffer(chunkSize + 1, 0);
		int bytesReceived = recv(clientSocket, buffer.data(), chunkSize, 0);
		if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
			std::cerr << "Error in receiving chunked data." << std::endl;
			return -1;
		}

		buffer[chunkSize] = '\0';
		fileHandler.appendDataToFile(pathToFile, buffer.data());
		totalReceived += bytesReceived;
	}

	return 0;
}

int DataStreamer::sendFileUsingChunks(const SOCKET& clientSocket, std::string&& pathToFile, int chunkSize) const {
	std::ifstream isize(pathToFile, std::ifstream::ate | std::ifstream::binary);
	long long size = isize.tellg();

	if (send(clientSocket, reinterpret_cast<const char*>(&size), sizeof(long long), 0) == SOCKET_ERROR) {
		std::cerr << "Failed to send total size." << std::endl;
		return -1;
	}

	std::ifstream ifile(move(pathToFile), std::ifstream::binary);
	long long totalSent = 0;

	if (ifile.good())
	{
		long long remaining, currentChunkSize;

		while (totalSent < size)
		{
			remaining = size - totalSent;
			currentChunkSize = (remaining < chunkSize) ? remaining : chunkSize;

			std::vector<char> buffer(currentChunkSize, 0);

			std::cout << "Sent chunk of size: " << currentChunkSize << std::endl;
			if (send(clientSocket, reinterpret_cast<const char*>(&currentChunkSize), sizeof(long long), 0) == SOCKET_ERROR) {
				std::cerr << "Failed to send chunk size." << std::endl;
				return -1;
			}

			ifile.read(buffer.data(), currentChunkSize);
			std::streamsize s = ((ifile) ? currentChunkSize : ifile.gcount());

			if (send(clientSocket, reinterpret_cast<char*>(buffer.data()), currentChunkSize, 0) == SOCKET_ERROR) {
				std::cerr << "Failed to send chunked data." << std::endl;
				return -1;
			}
			totalSent += currentChunkSize;
		}
		ifile.close();
		return 0;
	}
	else
	{
		std::cerr << "Error while reading the file\n";
		return -1;
	}

	isize.close();

	return 0;
}
int DataStreamer::sendChunkedData(const SOCKET& clientSocket, const char* data, int chunkSize) const {
	int dataSize = strlen(data);

	if (send(clientSocket, reinterpret_cast<const char*>(&dataSize), sizeof(int), 0) == SOCKET_ERROR) {
		std::cerr << "Failed to send total size." << std::endl;
		return -1;
	}

	if (send(clientSocket, reinterpret_cast<const char*>(&chunkSize), sizeof(int), 0) == SOCKET_ERROR) {
		std::cerr << "Failed to send chunk size." << std::endl;
		return -1;
	}

	int totalSent = 0;

	while (totalSent < dataSize) {
		int remaining = dataSize - totalSent;
		int currentChunkSize = (remaining < chunkSize) ? remaining : chunkSize;

		if (send(clientSocket, data + totalSent, currentChunkSize, 0) == SOCKET_ERROR) {
			std::cerr << "Failed to send chunked data." << std::endl;
			break;
		}

		totalSent += currentChunkSize;
	}

	return 0;
}
int DataStreamer::sendIntData(const SOCKET& clientSocket, int num) const {
	if (send(clientSocket, reinterpret_cast<const char*>(&num), sizeof(int32_t), 0) == SOCKET_ERROR) {
		std::cerr << "Failed to send int data." << std::endl;
		return -1;
	}

	return 0;
}
