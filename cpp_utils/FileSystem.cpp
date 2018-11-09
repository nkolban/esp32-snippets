/*
 * FileSystem.cpp
 *
 *  Created on: May 20, 2017
 *      Author: kolban
 */
#include <iostream>
#include <sstream>
#include <string>

#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <esp_log.h>

#include "FileSystem.h"

static const char* LOG_TAG = "FileSystem";

/**
 * @brief Dump a given directory to the log.
 * @param [in] path The path to the directory to dump.
 * @return N/A.
 */
void FileSystem::dumpDirectory(std::string path) {
	DIR* pDir = ::opendir(path.c_str());
	if (pDir == nullptr) {
		ESP_LOGD(LOG_TAG, "Unable to open directory: %s [errno=%d]", path.c_str(), errno);
		return;
	}
	struct dirent* pDirent;
	ESP_LOGD(LOG_TAG, "Directory dump of %s", path.c_str());
	while ((pDirent = readdir(pDir)) != nullptr) {
		std::string type;
		switch (pDirent->d_type) {
			case DT_UNKNOWN:
				type = "Unknown";
				break;
			case DT_REG:
				type = "Regular";
				break;
			case DT_DIR:
				type = "Directory";
				break;
			default:
				type = "Unknown";
				break;
		}
		ESP_LOGD(LOG_TAG, "Entry: d_ino: %d, d_name: %s, d_type: %s", pDirent->d_ino, pDirent->d_name, type.c_str());
	}
	::closedir(pDir);
} // dumpDirectory


/**
 * @brief Get the contents of a directory.
 * @param [in] path The path to the directory.
 * @return A vector of Files in the directory.
 */
std::vector<File> FileSystem::getDirectoryContents(std::string path) {
	std::vector<File> ret;
	DIR* pDir = ::opendir(path.c_str());
	if (pDir == nullptr) {
		ESP_LOGE(LOG_TAG, "getDirectoryContents:: Unable to open directory: %s [errno=%d]", path.c_str(), errno);
		return ret;
	}
	struct dirent* pDirent;
	ESP_LOGD(LOG_TAG, "Directory dump of %s", path.c_str());
	while ((pDirent = readdir(pDir)) != nullptr) {
		File file(path +"/" + std::string(pDirent->d_name), pDirent->d_type);
		ret.push_back(file);
	}
	::closedir(pDir);
	return ret;
} // getDirectoryContents


/**
 * @brief Does the path refer to a directory?
 * @param [in] path The path to the directory.
 */
bool FileSystem::isDirectory(std::string path) {
	struct stat statBuf;
	int rc = stat(path.c_str(), &statBuf);
	if (rc != 0) return false;
	return S_ISDIR(statBuf.st_mode);
} // isDirectory



/**
 * @brief Create a directory
 * @param [in] path The directory to create.
 * @return N/A.
 */
int FileSystem::mkdir(std::string path) {
	ESP_LOGD(LOG_TAG, ">> mkdir: %s", path.c_str());
	int rc = ::mkdir(path.c_str(), 0);
	if (rc != 0) {
		ESP_LOGE(LOG_TAG, "mkdir: errno=%d %s", errno, strerror(errno));
		rc = errno;
	}
	return rc;
} // mkdir


/**
 * @brief Return the constituent parts of the path.
 * If we imagine a path as composed of parts separated by slashes, then this function
 * returns a vector composed of the parts.  For example:
 *
 * ```
 * /x/y/z
 * ```
 * will break out to:
 *
 * ```
 * path[0] = ""
 * path[1] = "x"
 * path[2] = "y"
 * path[3] = "z"
 * ```
 *
 * @return A vector of the constituent parts of the path.
 */
std::vector<std::string> FileSystem::pathSplit(std::string path) {
	std::istringstream stream(path);
	std::vector<std::string> ret;
	std::string pathPart;
	while (std::getline(stream, pathPart, '/')) {
		ret.push_back(pathPart);
	}
	// Debug
	for (int i = 0; i < ret.size(); i++) {
		ESP_LOGD(LOG_TAG, "part[%d]: %s", i, ret[i].c_str());
	}
	return ret;
} // pathSplit


/**
 * @brief Remove a file from the file system.
 * @param [in] path The path to the file to be removed.
 * @return The return code of the underlying call.
 */
int FileSystem::remove(std::string path) {
	int rc = ::unlink(path.c_str());
	if (rc != 0) {
		ESP_LOGE(LOG_TAG, "unlink: errno=%d", errno);
		rc = errno;
	}
	return rc;
} // remove
