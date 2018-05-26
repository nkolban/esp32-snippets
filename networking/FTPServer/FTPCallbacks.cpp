#include "FTPServer.h"
#include <stdint.h>
#include <fstream>
#include <dirent.h>


/**
 * Called at the start of a STOR request.  The file name is the name of the file the client would like to
 * save.
 */
void FTPFileCallbacks::onStoreStart(std::string fileName) {
	printf(">> FTPFileCallbacks::onStoreStart: fileName=%s\n", fileName.c_str());
	m_storeFile.open(fileName, std::ios::binary);                        // Open the file for writing.
	if (m_storeFile.fail()) {
		throw FTPServer::FileException();
	}
	printf("<< FTPFileCallbacks::onStoreStart\n");
} // FTPFileCallbacks#onStoreStart


/**
 * Called when the client presents a new chunk of data to be saved.
 */
size_t FTPFileCallbacks::onStoreData(uint8_t* data, size_t size) {
	printf(">> FTPFileCallbacks::onStoreData: size=%ld\n", size);
	m_storeFile.write((char *)data, size);                               // Store data received.
	printf("<< FTPFileCallbacks::onStoreData: size=%ld\n", size);
	return size;
} // FTPFileCallbacks#onStoreData


/**
 * Called at the end of a STOR request.  This indicates that the client has completed its transmission of the
 * file.
 */
void FTPFileCallbacks::onStoreEnd() {
	printf(">> FTPFileCallbacks::onStoreEnd\n");
	m_storeFile.close();                                                 // Close the open file.
	printf("<< FTPFileCallbacks::onStoreEnd\n");
} // FTPFileCallbacks#onStoreEnd


/**
 * Called when the client requests retrieval of a file.
 */
void FTPFileCallbacks::onRetrieveStart(std::string fileName) {
	printf(">> FTPFileCallbacks::onRetrieveStart: fileName=%s\n", fileName.c_str());
	m_byteCount = 0;
	m_retrieveFile.open(fileName, std::ios::binary);
	if (m_retrieveFile.fail()) {
		printf("<< FTPFileCallbacks::onRetrieveStart: ***FileException***\n");
		throw FTPServer::FileException();
	}
	printf("<< FTPFileCallbacks::onRetrieveStart\n");
} // FTPFileCallbacks#onRetrieveStart


/**
 * Called when the client is ready to receive the next piece of the file.  To indicate that there
 * is no more data to send, return a size of 0.
 * @param data The data buffer that we can fill to return data back to the client.
 * @param size The maximum size of the data buffer that we can populate.
 * @return The size of data being returned.  Return 0 to indicate that there is no more data to return.
 */
size_t FTPFileCallbacks::onRetrieveData(uint8_t* data, size_t size) {
	printf(">> FTPFileCallbacks::onRetrieveData\n");
	m_retrieveFile.read((char *)data, size);
	size_t readSize = m_retrieveFile.gcount();
	m_byteCount += readSize;
	printf("<< FTPFileCallbacks::onRetrieveData: sizeRead=%ld\n", readSize);
	return m_retrieveFile.gcount();  // Return the number of bytes read.
} // FTPFileCallbacks#onRetrieveData


/**
 * Called when the retrieval has been completed.
 */
void FTPFileCallbacks::onRetrieveEnd() {
	printf(">> FTPFileCallbacks::onRetrieveEnd\n");
	m_retrieveFile.close();
	printf("<< FTPFileCallbacks::onRetrieveEnd: bytesTransmitted=%d\n", m_byteCount);
} // FTPFileCallbacks#onRetrieveEnd


/**
 * Return a list of files in the file system.
 * @return a list of files in the file system.
 */
std::string FTPFileCallbacks::onDir() {

	DIR* dir = opendir(FTPServer::getCurrentDirectory().c_str());
	std::stringstream ss;
	while(1) {
		struct dirent* pDirentry = readdir(dir);
		if (pDirentry == nullptr) {
			break;
		}
		ss << pDirentry->d_name << "\r\n";
	}
	closedir(dir);
	return ss.str();
} // FTPFileCallbacks#onDir


/// ---- END OF FTPFileCallbacks


void FTPCallbacks::onStoreStart(std::string fileName) {
	printf(">> FTPCallbacks::onStoreStart: fileName=%s\n", fileName.c_str());
	printf("<< FTPCallbacks::onStoreStart\n");
} // FTPCallbacks#onStoreStart


size_t FTPCallbacks::onStoreData(uint8_t* data, size_t size) {
	printf(">> FTPCallbacks::onStoreData: size=%ld\n", size);
	printf("<< FTPCallbacks::onStoreData\n");
	return 0;
} // FTPCallbacks#onStoreData


void FTPCallbacks::onStoreEnd() {
	printf(">> FTPCallbacks::onStoreEnd\n");
	printf("<< FTPCallbacks::onStoreEnd\n");
} // FTPCallbacks#onStoreEnd


void FTPCallbacks::onRetrieveStart(std::string fileName) {
	printf(">> FTPCallbacks::onRetrieveStart\n");
	printf("<< FTPCallbacks::onRetrieveStart\n");
} // FTPCallbacks#onRetrieveStart


size_t FTPCallbacks::onRetrieveData(uint8_t *data, size_t size) {
	printf(">> FTPCallbacks::onRetrieveData\n");
	printf("<< FTPCallbacks::onRetrieveData: 0\n");
	return 0;
} // FTPCallbacks#onRetrieveData


void FTPCallbacks::onRetrieveEnd() {
printf(">> FTPCallbacks::onRetrieveEnd\n");
printf("<< FTPCallbacks::onRetrieveEnd\n");
} // FTPCallbacks#onRetrieveEnd


std::string FTPCallbacks::onDir() {
	printf(">> FTPCallbacks::onDir\n");
	printf("<< FTPCallbacks::onDir\n");
	return "";
} // FTPCallbacks#onDir

FTPCallbacks::~FTPCallbacks() {

} // FTPCallbacks#~FTPCallbacks
