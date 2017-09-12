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
#include "JSON.h"
static const char* LOG_TAG = "WebSocketFileTransfer";

#include "WebSocketFileTransfer.h"

#undef close

WebSocketFileTransfer::WebSocketFileTransfer() {
	m_fileName   = "";
	m_length     = 0;
	m_pWebSocket = nullptr;
}

WebSocketFileTransfer::~WebSocketFileTransfer() {
	// TODO Auto-generated destructor stub
}


// Hide the class in an un-named namespace
namespace {

class FileTransferWebSocketHandler : public WebSocketHandler {
public:
	FileTransferWebSocketHandler() {
		m_fileName     = "";
		m_fileLength   = 0;
		m_sizeReceived = 0;
		m_active       = false;
	}

private:
	std::string   m_fileName;      // The name of the file we are receiving.
	uint32_t      m_fileLength;    // We may optionally receive a file length.
	uint32_t      m_sizeReceived;  // The size of the data actually received so far.
	bool          m_active;        // Are we actively processing a file.
	std::ofstream m_ofStream;

	virtual void onMessage(WebSocketInputStreambuf* pWebSocketInputStreambuf) {
		ESP_LOGD("FileTransferWebSocketHandler", ">> onMessage");
		if (!m_active) {
			std::stringstream buffer;
			buffer << pWebSocketInputStreambuf;
			ESP_LOGD("FileTransferWebSocketHandler", "Data read: %s", buffer.str().c_str());
			JsonObject jo = JSON::parseObject(buffer.str());
			m_fileName    = jo.getString("name");
			if (jo.hasItem("length")) {
				m_fileLength  = jo.getInt("length");
			}
			std::string fileName = "/spiflash/" + m_fileName;
			if (m_fileName.length() > 0 && m_fileName.substr(m_fileName.size()-1)=="/") {
				ESP_LOGD("FileTransferWebSocketHandler", "Is a directory!!");
				fileName = fileName.substr(0, fileName.size()-1);
				struct stat statbuf;
				if (stat(fileName.c_str(), &statbuf) == 0) {
					if (S_ISREG(statbuf.st_mode)) {
						ESP_LOGE("FileTransferWebSocketHandler", "File already exists and is a file not a directory!");
					}
				}
				if (mkdir(fileName.c_str(), 0) != 0) {
					ESP_LOGE("FileTransferWebSocketHandler", "Failed to make directory \"%s\", error: %s", fileName.c_str(), strerror(errno));
				}
			} else {
				m_ofStream.open(fileName, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
				if (!m_ofStream.is_open()) {
					ESP_LOGE("FileTransferWebSocketHandler", "Failed to open file %s", m_fileName.c_str());
				}
			}
			m_active      = true;
			ESP_LOGD("FileTransferWebSocketHandler", "Filename: %s, length: %d", fileName.c_str(), m_fileLength);
		} else {
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


	virtual void onClose() {
		ESP_LOGD("FileTransferWebSocketHandler", ">> onClose: fileName: %s, sizeReceived: %d", m_fileName.c_str(), m_sizeReceived);
		if (m_fileLength > 0 && m_sizeReceived != m_fileLength) {
			ESP_LOGD("FileTransferWebSocketHandler",
				"ERROR: Transfer finished but we received total of %d bytes and expected %d bytes!", m_sizeReceived, m_fileLength);
		}
		m_ofStream.close();
	} // onClose
}; // FileTransferWebSocketHandler

} // End un-named namespace

void WebSocketFileTransfer::start(WebSocket* pWebSocket) {
	ESP_LOGD(LOG_TAG, ">> start");
	pWebSocket->setHandler(new FileTransferWebSocketHandler());
} // start
