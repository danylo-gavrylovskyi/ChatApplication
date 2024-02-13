#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>

#include <mutex>

#include <string>

class IFileHandler {
public:
	virtual ~IFileHandler() {};

	virtual int appendDataToFile(const std::string& pathToFile, const char* buffer) const = 0;
	virtual int deleteFile(const std::string& pathToFile) const = 0;
	virtual char* getFileInfo(std::string&& pathToFile) const = 0;
	virtual int doesDirectoryExist(const std::string& pathToFile) const = 0;
	virtual int createDirectory(const std::string& directory) const = 0;
};

class FileHandler : public IFileHandler {
	std::mutex mtx;
public:
	FileHandler();
	FileHandler(const FileHandler&) = delete;
	FileHandler(FileHandler&&) = delete;

	int appendDataToFile(const std::string& pathToFile, const char* buffer) const override;
	int deleteFile(const std::string& pathToFile) const override;
	char* getFileInfo(std::string&& pathToFile) const override;
	int doesDirectoryExist(const std::string& pathToFile) const override;
	int createDirectory(const std::string& directory) const override;
};
