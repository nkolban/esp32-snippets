/*
 * FileSystem.cpp
 *
 *  Created on: May 20, 2017
 *      Author: kolban
 */
#include <string>
#include <esp_log.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include "FileSystem.h"

static char tag[] = "FileSystem";

FileSystem::FileSystem() {
	// TODO Auto-generated constructor stub

}

FileSystem::~FileSystem() {
	// TODO Auto-generated destructor stub
}


/**
 * @brief Dump a given directory to the log.
 * @param [in] path The path to the directory to dump.
 * @return N/A.
 */
void FileSystem::dumpDirectory(std::string path) {
	DIR *pDir = ::opendir(path.c_str());
	if (pDir == nullptr) {
		ESP_LOGD(tag, "Unable to open directory: %s [errno=%d]", path.c_str(), errno);
		return;
	}
	struct dirent *pDirent;
	ESP_LOGD(tag, "Directory dump of %s", path.c_str());
	while((pDirent = readdir(pDir)) != nullptr) {
		std::string type;
		switch(pDirent->d_type) {
		case DT_UNKNOWN:
			type = "Unknown";
			break;
		case DT_REG:
			type = "Regular";
			break;
		case DT_DIR:
			type = "Directory";
			break;
		}
		ESP_LOGD(tag, "Entry: d_ino: %d, d_name: %s, d_type: %s", pDirent->d_ino, pDirent->d_name, type.c_str());
	}
	::closedir(pDir);
} // dumpDirectory

int FileSystem::mkdir(std::string path) {
	int rc = ::mkdir(path.c_str(), 0);
	if (rc != 0) {
		ESP_LOGE(tag, "mkdir: errno=%d", errno);
	}
	return rc;
}
