#include "FileHandler.h"

FileHandler::FileHandler(){}

int FileHandler::deleteFile(const std::string& pathToFile) const
{
	if (std::filesystem::remove(pathToFile)) return 0;
	else return -1;
}

char* FileHandler::getFileInfo(std::string&& pathToFile) const
{
	char* fileInfo = new char[255];
	
	struct stat fInfo;
	if (stat(move(pathToFile).c_str(), &fInfo) != 0) {
		std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(this->mtx));
		std::cerr << "Error: " << strerror(errno) << '\n';
	}

	sprintf(fileInfo, "%lld", (long long)fInfo.st_size);
	strcat(fileInfo, ";");
	strcat(fileInfo, std::ctime(&fInfo.st_ctime));
	strcat(fileInfo, ";");
	strcat(fileInfo, std::ctime(&fInfo.st_mtime));

	return fileInfo;
}

int FileHandler::appendDataToFile(const std::string& pathToFile, const char* buffer) const
{
	std::ofstream file;
	file.open(pathToFile, std::ios::out | std::ios::app);
	if (file.fail()) {
		throw std::ios_base::failure(std::strerror(errno));
		return -1;
	}

	file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);

	file << buffer;
	file.close();
	return 0;
}

int FileHandler::doesDirectoryExist(const std::string& pathToFile) const
{
	struct stat sb;

	if (stat(pathToFile.c_str(), &sb) == 0) {
		return 0;
	}
	else {
		return -1;
	}
}

int FileHandler::createDirectory(const std::string& directory) const
{
	try {
		std::filesystem::create_directory(directory);
		std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(this->mtx));
		std::cout << "Folder created successfully." << std::endl;
		return 0;
	}
	catch (const std::filesystem::filesystem_error& e) {
		std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(this->mtx));
		std::cerr << "Failed to create folder: " << e.what() << std::endl;
		return -1;
	}
}
