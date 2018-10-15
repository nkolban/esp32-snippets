#include "FTPServer.h"
#include <stdint.h>
#include <fstream>
#include <dirent.h>
#include <esp_log.h>

static const char* LOG_TAG = "FTPCallbacks";
/**
 * Called at the start of a STOR request.  The file name is the name of the file the client would like to
 * save.
 */
void FTPFileCallbacks::onStoreStart(std::string fileName) {
	ESP_LOGD(LOG_TAG, ">> FTPFileCallbacks::onStoreStart: fileName=%s", fileName.c_str());
	m_storeFile.open(fileName, std::ios::binary);                        // Open the file for writing.
	if (m_storeFile.fail()) {
		throw FTPServer::FileException();
	}
	ESP_LOGD(LOG_TAG,"<< FTPFileCallbacks::onStoreStart");
} // FTPFileCallbacks#onStoreStart


/**
 * Called when the client presents a new chunk of data to be saved.
 */
size_t FTPFileCallbacks::onStoreData(uint8_t* data, size_t size) {
	ESP_LOGD(LOG_TAG,">> FTPFileCallbacks::onStoreData: size=%d", size);
	m_storeFile.write((char*) data, size);							   // Store data received.
	ESP_LOGD(LOG_TAG,"<< FTPFileCallbacks::onStoreData: size=%d", size);
	return size;
} // FTPFileCallbacks#onStoreData


/**
 * Called at the end of a STOR request.  This indicates that the client has completed its transmission of the
 * file.
 */
void FTPFileCallbacks::onStoreEnd() {
	ESP_LOGD(LOG_TAG,">> FTPFileCallbacks::onStoreEnd");
	m_storeFile.close();                                                 // Close the open file.
	ESP_LOGD(LOG_TAG,"<< FTPFileCallbacks::onStoreEnd");
} // FTPFileCallbacks#onStoreEnd


/**
 * Called when the client requests retrieval of a file.
 */
void FTPFileCallbacks::onRetrieveStart(std::string fileName) {
	ESP_LOGD(LOG_TAG,">> FTPFileCallbacks::onRetrieveStart: fileName=%s", fileName.c_str());
	m_byteCount = 0;
	m_retrieveFile.open(fileName, std::ios::binary);
	if (m_retrieveFile.fail()) {
		ESP_LOGD(LOG_TAG,"<< FTPFileCallbacks::onRetrieveStart: ***FileException***");
		throw FTPServer::FileException();
	}
	ESP_LOGD(LOG_TAG,"<< FTPFileCallbacks::onRetrieveStart");
} // FTPFileCallbacks#onRetrieveStart


/**
 * Called when the client is ready to receive the next piece of the file.  To indicate that there
 * is no more data to send, return a size of 0.
 * @param data The data buffer that we can fill to return data back to the client.
 * @param size The maximum size of the data buffer that we can populate.
 * @return The size of data being returned.  Return 0 to indicate that there is no more data to return.
 */
size_t FTPFileCallbacks::onRetrieveData(uint8_t* data, size_t size) {
	ESP_LOGD(LOG_TAG,">> FTPFileCallbacks::onRetrieveData");
	m_retrieveFile.read((char*) data, size);
	size_t readSize = m_retrieveFile.gcount();
	m_byteCount += readSize;
	ESP_LOGD(LOG_TAG,"<< FTPFileCallbacks::onRetrieveData: sizeRead=%d", readSize);
	return m_retrieveFile.gcount();  // Return the number of bytes read.
} // FTPFileCallbacks#onRetrieveData


/**
 * Called when the retrieval has been completed.
 */
void FTPFileCallbacks::onRetrieveEnd() {
	ESP_LOGD(LOG_TAG,">> FTPFileCallbacks::onRetrieveEnd");
	m_retrieveFile.close();
	ESP_LOGD(LOG_TAG,"<< FTPFileCallbacks::onRetrieveEnd: bytesTransmitted=%d", m_byteCount);
} // FTPFileCallbacks#onRetrieveEnd


/**
 * Return a list of files in the file system.
 * @return a list of files in the file system.
 */
std::string FTPFileCallbacks::onDir() {
	DIR* dir = opendir(FTPServer::getCurrentDirectory().c_str());
	std::stringstream ss;
	while (true) {
		struct dirent* pDirentry = readdir(dir);
		if (pDirentry == nullptr) break;
		ss << pDirentry->d_name << "\r\n";
	}
	closedir(dir);
	return ss.str();
} // FTPFileCallbacks#onDir


/// ---- END OF FTPFileCallbacks


void FTPCallbacks::onStoreStart(std::string fileName) {
	ESP_LOGD(LOG_TAG,">> FTPCallbacks::onStoreStart: fileName=%s", fileName.c_str());
	ESP_LOGD(LOG_TAG,"<< FTPCallbacks::onStoreStart");
} // FTPCallbacks#onStoreStart


size_t FTPCallbacks::onStoreData(uint8_t* data, size_t size) {
	ESP_LOGD(LOG_TAG,">> FTPCallbacks::onStoreData: size=%d", size);
	ESP_LOGD(LOG_TAG,"<< FTPCallbacks::onStoreData");
	return 0;
} // FTPCallbacks#onStoreData


void FTPCallbacks::onStoreEnd() {
	ESP_LOGD(LOG_TAG,">> FTPCallbacks::onStoreEnd");
	ESP_LOGD(LOG_TAG,"<< FTPCallbacks::onStoreEnd");
} // FTPCallbacks#onStoreEnd


void FTPCallbacks::onRetrieveStart(std::string fileName) {
	ESP_LOGD(LOG_TAG,">> FTPCallbacks::onRetrieveStart");
	ESP_LOGD(LOG_TAG,"<< FTPCallbacks::onRetrieveStart");
} // FTPCallbacks#onRetrieveStart


size_t FTPCallbacks::onRetrieveData(uint8_t* data, size_t size) {
	ESP_LOGD(LOG_TAG,">> FTPCallbacks::onRetrieveData");
	ESP_LOGD(LOG_TAG,"<< FTPCallbacks::onRetrieveData: 0");
	return 0;
} // FTPCallbacks#onRetrieveData


void FTPCallbacks::onRetrieveEnd() {
	ESP_LOGD(LOG_TAG,">> FTPCallbacks::onRetrieveEnd");
	ESP_LOGD(LOG_TAG,"<< FTPCallbacks::onRetrieveEnd");
} // FTPCallbacks#onRetrieveEnd


std::string FTPCallbacks::onDir() {
	ESP_LOGD(LOG_TAG,">> FTPCallbacks::onDir");
	ESP_LOGD(LOG_TAG,"<< FTPCallbacks::onDir");
	return "";
} // FTPCallbacks#onDir

FTPCallbacks::~FTPCallbacks() {

} // FTPCallbacks#~FTPCallbacks
