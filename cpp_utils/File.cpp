/*
 * File.cpp
 *
 *  Created on: Jun 1, 2017
 *      Author: kolban
 */

#include "File.h"
#include <sys/stat.h>
#include <esp_log.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include "GeneralUtils.h"

static const char* LOG_TAG = "File";
/**
 * @brief Construct a file.
 * @param [in] name The name of the file.
 * @param [in] type The type of the file (DT_REGULAR, DT_DIRECTORY or DT_UNKNOWN).
 */
File::File(std::string path, uint8_t type) {
	m_path = path;
	m_type = type;
} // File


/**
 * @brief Retrieve the content of the file.
 * @param [in] base64Encode Should we base64 encode the content?
 * @return The content of the file.
 */
std::string File::getContent(bool base64Encode) {
	uint32_t size = length();
	ESP_LOGD(LOG_TAG, "File:: getContent(), path=%s, length=%d", m_path.c_str(), size);
	if (size == 0) return "";
	uint8_t* pData = (uint8_t*) malloc(size);
	if (pData == nullptr) {
		ESP_LOGE(LOG_TAG, "getContent: Failed to allocate memory");
		return "";
	}
	FILE* file = fopen(m_path.c_str(), "r");
	fread(pData, size, 1, file);
	fclose(file);
	std::string ret((char *)pData, size);
	free(pData);
	if (base64Encode) {
		std::string encoded;
		GeneralUtils::base64Decode(ret, &encoded);
		return encoded;
	}
	return ret;
} // getContent


/**
 * @brief Retrieve the content of the file.
 * @param [in] offset The file offset to read from.
 * @param [in] readSize The number of bytes to read.
 * @return The content of the file.
 */
std::string File::getContent(uint32_t offset, uint32_t readSize) {
	uint32_t fileSize = length();
	ESP_LOGD(LOG_TAG, "File:: getContent(), name=%s, fileSize=%d, offset=%d, readSize=%d",
		m_path.c_str(), fileSize, offset, readSize);
	if (fileSize == 0 || offset > fileSize) return "";
	uint8_t* pData = (uint8_t*) malloc(readSize);
	if (pData == nullptr) {
		ESP_LOGE(LOG_TAG, "getContent: Failed to allocate memory");
		return "";
	}
	FILE* file = fopen(m_path.c_str(), "r");
	fseek(file, offset, SEEK_SET);
	size_t bytesRead = fread(pData, 1, readSize, file);
	fclose(file);
	std::string ret((char*) pData, bytesRead);
	free(pData);
	return ret;
} // getContent


std::string File::getPath() {
	return m_path;
}
/**
 * @brief Get the name of the file.
 * @return The name of the file.
 */
std::string File::getName() {
	size_t pos = m_path.find_last_of('/');
	if (pos == std::string::npos) return m_path;
	return m_path.substr(pos + 1);
} // getName


/**
 * @brief Get the type of the file.
 * The type of a file can be DT_REGULAR, DT_DIRECTORY or DT_UNKNOWN.
 * @return The type of the file.
 */
uint8_t File::getType() {
	return m_type;
} // getName


/**
 * @brief Get the length of the file in bytes.
 * @return The length of the file in bytes.
 */
uint32_t File::length() {
	struct stat buf;
	int rc = stat(m_path.c_str(), &buf);
	if (rc != 0 || S_ISDIR(buf.st_mode)) return 0;
	return (uint32_t) buf.st_size;
} // length


/**
 * @brief Determine if the type of the file is a directory.
 * @return True if the file is a directory.
 */
bool File::isDirectory() {
	struct stat buf;
	int rc = stat(m_path.c_str(), &buf);
	if (rc != 0) return false;
	return S_ISDIR(buf.st_mode);
} // isDirectory
