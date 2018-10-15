/*
 * WebSocketFileTransfer.cpp
 *
 *  Created on: Sep 9, 2017
 *      Author: kolban
 */
#include <sstream>
#include <fstream>
#include <esp_log.h>
#include <sys/stat.h>
#include "GeneralUtils.h"
#include "JSON.h"
static const char* LOG_TAG = "WebSocketFileTransfer";

#include "WebSocketFileTransfer.h"

#undef close

/**
 * @brief Constructor
 * @param [in] rootPath The path prefix for new files.
 */
WebSocketFileTransfer::WebSocketFileTransfer(std::string rootPath) {
	// If the root path doesn't end with a '/', add one.
	if (!GeneralUtils::endsWith(rootPath, '/')) {
		rootPath += '/';
	}
	m_rootPath   = rootPath;
	m_pWebSocket = nullptr;
	if (rootPath.empty()) {
		ESP_LOGE(LOG_TAG, "Root path can not be empty");
	} else if (m_rootPath.substr(m_rootPath.size() - 1) != "/") {
		ESP_LOGE(LOG_TAG, "Root path must end with a \"/\"");
	}
} // WebSocketFileTransfer


// Hide the class in an un-named namespace
namespace {

/**
 * @brief Transfer handler.
 */
class FileTransferWebSocketHandler : public WebSocketHandler {
public:
	FileTransferWebSocketHandler(std::string rootPath) {
		m_fileName     = "";
		m_fileLength   = 0;
		m_sizeReceived = 0;
		m_active       = false;
		m_rootPath     = rootPath;
	} // FileTransferWebSocketHandler

	/**
	 * @brief Handler for the message received over the web socket.
	 */
	virtual void onMessage(WebSocketInputStreambuf* pWebSocketInputStreambuf) {
		ESP_LOGD("FileTransferWebSocketHandler", ">> onMessage");
		// Test to see if we are currently active.  If not, this is the start of a transfer.
		if (!m_active) {
			ESP_LOGD("FileTransferWebSocketHandler", "Not yet active!");
			// Read a chunk of data into memory.
			std::stringstream buffer;
			buffer << pWebSocketInputStreambuf;

			ESP_LOGD("FileTransferWebSocketHandler", "Data read: %s", buffer.str().c_str());

			// We expect the first chunk received to be a JSON object that contains
			// {
			//    "name":   <fileName>,      // Name of file to create.
			//    "length": <lengthOfFile>   // Length of file. Optional.
			// }
			JsonObject jo = JSON::parseObject(buffer.str());
			m_fileName	= jo.getString("name");
			assert(m_fileName.length() > 0); // Doesn't make any sense to receive a zero length file name.
			if (jo.hasItem("length")) {
				m_fileLength  = jo.getInt("length");
			}
			std::string fileName = m_rootPath + m_fileName;
			ESP_LOGD("FileTransferWebSocketHandler", "Target file is %s", fileName.c_str());

			// If the file to create ends in a "/" then we are being asked to create a directory.
			if (m_fileName.substr(m_fileName.size() - 1) == "/") {
				ESP_LOGD("FileTransferWebSocketHandler", "Is a directory!!");
				fileName = fileName.substr(0, fileName.size() - 1);   // Remove the trailing slash
				struct stat statbuf;
				if (stat(fileName.c_str(), &statbuf) == 0) {
					if (S_ISREG(statbuf.st_mode)) {
						ESP_LOGE("FileTransferWebSocketHandler", "File already exists and is a file not a directory!");
					}
				} // Stat the directory we are trying to create.
				else { // The stat on the path failed ... which is ok, as it likely means that the entry didn't exist.
					if (mkdir(fileName.c_str(), 0) != 0) {
						ESP_LOGE("FileTransferWebSocketHandler", "Failed to make directory \"%s\", error: %s", fileName.c_str(), strerror(errno));
					}
				}
			}
			// We are NOT creating a directory but are instead creating a file.
			else {
				m_ofStream.open(fileName, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
				if (!m_ofStream.is_open()) {
					ESP_LOGE("FileTransferWebSocketHandler", "Failed to open file %s for writing", m_fileName.c_str());
					return;
				}
			}
			m_active = true;
			ESP_LOGD("FileTransferWebSocketHandler", "Filename: %s, length: %d", fileName.c_str(), m_fileLength);
		} // !active --- Not active
		else {
			// We are about to receive a chunk of file
			m_ofStream << pWebSocketInputStreambuf;
			/*
			std::stringstream bufferStream;
			bufferStream << pWebSocketInputRecordStreambuf;
			m_sizeReceived += bufferStream.str().length();
			ESP_LOGD("FileTransferWebSocketHandler", "Received %d bytes of file data", bufferStream.str().length());
			if (m_fileLength > 0 && m_sizeReceived > m_fileLength) {
				ESP_LOGD("FileTransferWebSocketHandler",
					"ERROR: Received a total of %d bytes when only %d bytes expected!", m_sizeReceived, m_fileLength);
			}
			*/
		}
	} // onMessage

	/**
	 * @brief Handle a close event on the web socket.
	 */
	virtual void onClose() {
		ESP_LOGD("FileTransferWebSocketHandler", ">> onClose: fileName: %s, sizeReceived: %d", m_fileName.c_str(), m_sizeReceived);
		if (m_fileLength > 0 && m_sizeReceived != m_fileLength) {
			ESP_LOGD("FileTransferWebSocketHandler",
				"ERROR: Transfer finished but we received total of %d bytes and expected %d bytes!", m_sizeReceived, m_fileLength);
		}
		if (m_ofStream.is_open()) {
			m_ofStream.close();   // Close the file now that we have finished writing to it.
		}
		delete this;   // Delete ourselves.
	} // onClose

private:
	std::string   m_fileName;	  // The name of the file we are receiving.
	uint32_t      m_fileLength;	// We may optionally receive a file length.
	uint32_t      m_sizeReceived;  // The size of the data actually received so far.
	bool          m_active;		// Are we actively processing a file.
	std::ofstream m_ofStream;	  // The file stream we are writing to when active.
	std::string   m_rootPath;	  // The root path for file names.

}; // FileTransferWebSocketHandler

} // End un-named namespace


void WebSocketFileTransfer::start(WebSocket* pWebSocket) {
	ESP_LOGD(LOG_TAG, ">> start");
	pWebSocket->setHandler(new FileTransferWebSocketHandler(m_rootPath));
} // start


