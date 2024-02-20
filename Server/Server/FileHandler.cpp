#include "FileHandler.h"

FileHandler::FileHandler(){}

int FileHandler::deleteFile(const std::string& pathToFile) const
{
	if (std::filesystem::remove(pathToFile)) return 0;
	else return -1;
}

int FileHandler::appendDataToFile(const std::string& pathToFile, const char* buffer) const
{
	std::ofstream file;
	file.open(pathToFile, std::ios::out | std::ios::app);
	if (file.fail()) {
		std::cerr << "Error opening file: " << pathToFile << " - " << std::strerror(errno) << std::endl;
		throw std::ios_base::failure("Failed to open file");
	}

	file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);

	file << buffer;
	file.close();
	return 0;
}

std::string FileHandler::getFileSize(const std::string& pathToFile) const {
	std::filesystem::path path(pathToFile);
	std::uintmax_t sizeInBytes = std::filesystem::file_size(path);

	const char* units[] = { "B", "KB", "MB", "GB", "TB" };
	int unitIndex = 0;

	while (sizeInBytes >= 1024 && unitIndex < 4) {
		sizeInBytes /= 1024;
		++unitIndex;
	}

	std::ostringstream oss;
	oss << sizeInBytes << " " << units[unitIndex];
	return oss.str();
}
