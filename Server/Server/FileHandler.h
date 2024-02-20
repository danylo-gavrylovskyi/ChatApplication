#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>

#include <string>

class IFileHandler {
public:
	virtual ~IFileHandler() {};

	virtual int appendDataToFile(const std::string& pathToFile, const char* buffer) const = 0;
	virtual int deleteFile(const std::string& pathToFile) const = 0;
	virtual std::string getFileSize(const std::string& pathToFile) const = 0;
};

class FileHandler : public IFileHandler {
public:
	FileHandler();
	FileHandler(const FileHandler&) = delete;
	FileHandler(FileHandler&&) = delete;

	int appendDataToFile(const std::string& pathToFile, const char* buffer) const override;
	int deleteFile(const std::string& pathToFile) const override;
	std::string getFileSize(const std::string& pathToFile) const;
};
