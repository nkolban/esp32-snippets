/*
 * WebSocket.cpp
 *
 *  Created on: Sep 2, 2017
 *      Author: kolban
 */

#include <sstream>
#include "WebSocket.h"
#include "Task.h"
#include "GeneralUtils.h"
#include <esp_log.h>

static const char* LOG_TAG = "WebSocket";

// WebSocket op codes as found in a WebSocket frame.
static const int OPCODE_CONTINUE = 0x00;
static const int OPCODE_TEXT     = 0x01;
static const int OPCODE_BINARY   = 0x02;
static const int OPCODE_CLOSE    = 0x08;
static const int OPCODE_PING     = 0x09;
static const int OPCODE_PONG     = 0x0a;


// Structure definition for the WebSocket frame.
struct Frame {
	unsigned int opCode : 4; // [7:4]
	unsigned int rsv3   : 1; // [3]
	unsigned int rsv2   : 1; // [2]
	unsigned int rsv1   : 1; // [1]
	unsigned int fin    : 1; // [0]

	unsigned int len    : 7; // [7:1]
	unsigned int mask   : 1; // [0]
};


/**
 * @brief Dump the content of the frame for debugging.
 * @param [in] frame The frame to dump.
 */
static void dumpFrame(Frame frame) {
	std::ostringstream oss;
	oss << "Fin: " << frame.fin << ", OpCode: " << frame.opCode;
	switch(frame.opCode) {
		case OPCODE_BINARY: {
			oss << " BINARY";
			break;
		}
		case OPCODE_CONTINUE: {
			oss << " CONTINUE";
			break;
		}
		case OPCODE_CLOSE: {
			oss << " CLOSE";
			break;
		}
		case OPCODE_PING: {
			oss << " PING";
			break;
		}
		case OPCODE_PONG: {
			oss << " PONG";
			break;
		}
		case OPCODE_TEXT: {
			oss << " TEXT";
			break;
		}
		default: {
			oss << " Unknown";
			break;
		}
	}
	oss << ", Mask: " << frame.mask << ", len: " << frame.len;
	ESP_LOGD(LOG_TAG, "WebSocket frame: %s", oss.str().c_str());
} // dumpFrame


/**
 * @brief A task that will watch web socket inputs.
 */
class WebSocketReader: public Task {
	void run(void* data) {
		uint8_t buffer[1000];
		WebSocket *pWebSocket = (WebSocket*) data;
		// do something
		Socket peerSocket = pWebSocket->getSocket();

		ESP_LOGD(LOG_TAG, "Waiting on socket data for socket %s", peerSocket.toString().c_str());
		int length = peerSocket.receive_cpp(buffer, sizeof(buffer));
		ESP_LOGD(LOG_TAG, "Received data from web socket.  Length: %d", length);
		GeneralUtils::hexDump(buffer, length);
		dumpFrame(*(Frame *)buffer);

		// The following section parses the WebSocket frame.
		if (length > 0) {
			Frame* pFrame = (Frame*)buffer;
			uint32_t payloadLen = 0;
			uint8_t* pMask = nullptr;
			uint8_t* pData;
			if (pFrame->len < 126) {
				payloadLen = pFrame->len;
				pMask = buffer + 2;
			} else if (pFrame->len == 126) {
				payloadLen = *(uint16_t*)(buffer+2);
				pMask = buffer + 4;
			} else if (pFrame->len == 127) {
				ESP_LOGE(LOG_TAG, "Too much data!");
				return;
			}
			if (pFrame->mask == 1) {
				pData = pMask + 4;
				for (int i=0; i<payloadLen; i++) {
					*pData = *pData ^ pMask[i%4];
					pData++;
				}
				pData = pMask + 4;
			} else {
				pData = pMask;
			}
			std::string retData = std::string((char *)pData, payloadLen);

			ESP_LOGD(LOG_TAG, "Resulting payload:");
			GeneralUtils::hexDump(pData, payloadLen);
		}
	} // run
}; // WebSocketReader


void WebSocketHandler::onData(std::string data) {
} // onData


void WebSocketHandler::onError(std::string error) {
} // onError


WebSocket::WebSocket(Socket socket) {
	m_socket = socket;
	WebSocketReader *pWebSocketReader = new WebSocketReader();
	pWebSocketReader->start(this);
} // WebSocket


WebSocket::~WebSocket() {
} // ~WebSocket


/**
 * @brief Close the Web socket
 */
void WebSocket::close_cpp() {

} // close_cpp


/**
 * @brief Get the underlying socket for the websocket.
 * @return The socket associated with the Web socket.
 */
Socket WebSocket::getSocket() {
	return m_socket;
} // getSocket


/**
 * @brief Send data down the web socket
 * @param [in] data The data to send down the websocket.
 */
void WebSocket::send_cpp(std::string data) {
} // send_cpp


/**
 * @brief Set the Web socket handler associated with this Websocket.
 *
 * This will be the user supplied code that will be invoked to process incoming WebSocket
 * events.  An instance of WebSocketHandler is passed in.
 *
 */
void WebSocket::setHandler(WebSocketHandler handler) {
	m_webSocketHandler = handler;
} // setHandler
