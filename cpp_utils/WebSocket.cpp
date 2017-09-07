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
	// Byte 0
	uint8_t opCode : 4; // [7:4]
	uint8_t rsv3   : 1; // [3]
	uint8_t rsv2   : 1; // [2]
	uint8_t rsv1   : 1; // [1]
	uint8_t fin    : 1; // [0]

	// Byte 1
	uint8_t len    : 7; // [7:1]
	uint8_t mask   : 1; // [0]
};


/**
 * @brief Dump the content of the WebSocket frame for debugging.
 * @param [in] frame The frame to dump.
 */
static void dumpFrame(Frame frame) {
	std::ostringstream oss;
	oss << "Fin: " << (int)frame.fin << ", OpCode: " << (int)frame.opCode;
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
	oss << ", Mask: " << (int)frame.mask << ", len: " << (int)frame.len;
	ESP_LOGD(LOG_TAG, "WebSocket frame: %s", oss.str().c_str());
} // dumpFrame


/**
 * @brief A task that will watch web socket inputs.
 *
 * When a WebSocket is created it is created by the client requesting an HTTP protocol changed to WebSockets.
 * After the original Socket has been flagged as being a WebSocket, we must now start watching that socket for
 * incoming asynchronous events.  We spawn a task to do this.  This is the implementation of that task.
 */
class WebSocketReader: public Task {
	/**
	 * @brief Loop over the web socket waiting for new input.
	 * @param [in] data A pointer to an instance of the WebSocket.
	 */
	void run(void* data) {
		WebSocket *pWebSocket = (WebSocket*) data;
		ESP_LOGD("WebSocketReader", "WebSocketReader Task started, socket: %s", pWebSocket->getSocket().toString().c_str());
		uint8_t buffer[1000];

		Socket peerSocket = pWebSocket->getSocket();

		while(1) {
			ESP_LOGD("WebSocketReader", "Waiting on socket data for socket %s", peerSocket.toString().c_str());
			int length = peerSocket.receive_cpp(buffer, sizeof(buffer));
			if (length == -1 || length == 0) {
				ESP_LOGD(LOG_TAG, "Socket read error");
				pWebSocket->close_cpp();
				return;
			}
			ESP_LOGD("WebSocketReader", "Received data from web socket.  Length: %d", length);
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
				std::string payloadData = std::string((char *)pData, payloadLen);

				if (payloadLen == 0) {
					ESP_LOGD("WebSocketReader", "Web socket payload is empty");
				} else {
					ESP_LOGD("WebSocketReader", "Web socket payload, length=%d:", payloadLen);
					GeneralUtils::hexDump(pData, payloadLen);
				}

				WebSocketHandler *pWebSocketHandler = pWebSocket->getHandler();
				switch(pFrame->opCode) {
					case OPCODE_BINARY: {
						if (pWebSocketHandler != nullptr) {
							pWebSocketHandler->onMessage(payloadData);
						}
						break;
					}

					case OPCODE_CLOSE: {
						pWebSocket->m_receivedClose = true;
						if (pWebSocketHandler != nullptr) {
							pWebSocketHandler->onClose();
							pWebSocket->close_cpp();
						}
						break;
					}

					case OPCODE_CONTINUE: {
						break;
					}

					case OPCODE_PING: {
						break;
					}

					case OPCODE_PONG: {
						break;
					}

					case OPCODE_TEXT: {
						if (pWebSocketHandler != nullptr) {
							pWebSocketHandler->onMessage(payloadData);
						}
						break;
					}

					default: {
							ESP_LOGD("WebSocketReader", "Unknown opcode: %d", pFrame->opCode);
						break;
					}
				} // Switch opCode
			} // Length of data > 0
		} // While (1)
	} // run
}; // WebSocketReader


/**
 * @brief The default onClose handler.
 * If no over-riding handler is provided for the "close" event, this method is called.
 */
void WebSocketHandler::onClose() {
	ESP_LOGD(LOG_TAG, ">> WebSocketHandler:onClose()");
} // onClose


/**
 * @brief The default onData handler.
 * If no over-riding handler is provided for the "message" event, this method is called.
 */
void WebSocketHandler::onMessage(std::string data) {
	ESP_LOGD(LOG_TAG, ">> WebSocketHandler:onMessage(), length: %d", data.length());
	GeneralUtils::hexDump((uint8_t*)data.data(), data.length());
} // onData


/**
 * @brief The default onError handler.
 * If no over-riding handler is provided for the "error" event, this method is called.
 */
void WebSocketHandler::onError(std::string error) {
	ESP_LOGD(LOG_TAG, ">> WebSocketHandler:onError()");
} // onError


WebSocket::WebSocket(Socket socket) {
	m_receivedClose    = false;
	m_sentClose        = false;
	m_socket           = socket;
	m_pWebSockerReader = new WebSocketReader();
} // WebSocket


WebSocket::~WebSocket() {
} // ~WebSocket


/**
 * @brief Close the Web socket
 */
void WebSocket::close_cpp(uint16_t status, std::string message) {
	ESP_LOGD(LOG_TAG, ">> close_cpp(): status: %d, message: %s", status, message.c_str());
	if (m_sentClose) {
		m_socket.close_cpp();
		return;
	}
	m_sentClose = true;

	Frame frame;
	frame.fin    = 1;
	frame.rsv1   = 0;
	frame.rsv2   = 0;
	frame.rsv3   = 0;
	frame.opCode = OPCODE_CLOSE;
	frame.mask   = 0;
	frame.len = message.length() + 2;
	int rc = m_socket.send_cpp((uint8_t *)&frame, sizeof(frame));
	if (rc > 0) {
		rc = m_socket.send_cpp(status);
	}
	if (rc > 0) {
		m_socket.send_cpp(message);
	}
	if (m_receivedClose || rc == 0 || rc == -1) {
		m_socket.close_cpp();
	}
} // close_cpp


/**
 * @brief Get the current WebSocketHandler
 * A web socket handler is a user registered class instance that is called when an incoming
 * event received over the network needs to be handled by user code.
 */
WebSocketHandler* WebSocket::getHandler() {
	return &m_webSocketHandler;
} // getHandler


/**
 * @brief Get the underlying socket for the websocket.
 * @return The socket associated with the Web socket.
 */
Socket WebSocket::getSocket() {
	return m_socket;
} // getSocket


/**
 * @brief Send data down the web socket
 * See the WebSocket spec (RFC6455) section "6.1 Sending Data".
 * We build a WebSocket frame, send the frame followed by the data.
 * @param [in] data The data to send down the WebSocket.
 * @param [in] sendType The type of payload.  Either SEND_TYPE_TEXT or SEND_TYPE_BINARY.
 */
void WebSocket::send_cpp(std::string data, uint8_t sendType) {
	ESP_LOGD(LOG_TAG, ">> send_cpp: Length: %d", data.length());
	Frame frame;
	frame.fin    = 1;
	frame.rsv1   = 0;
	frame.rsv2   = 0;
	frame.rsv3   = 0;
	frame.opCode = sendType==SEND_TYPE_TEXT?OPCODE_TEXT:OPCODE_BINARY;
	frame.mask   = 0;
	if (data.length() < 126) {
		frame.len = data.length();
		m_socket.send_cpp((uint8_t *)&frame, sizeof(frame));
	} else {
		frame.len = 126;
		m_socket.send_cpp((uint8_t *)&frame, sizeof(frame));
		m_socket.send_cpp((uint16_t)data.length());
	}
	m_socket.send_cpp((uint8_t*)data.data(), data.length());
	ESP_LOGD(LOG_TAG, "<< send_cpp");
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


/**
 * @brief Start the WebSocket reader reading the socket.
 * When we have a new web socket, we want to start watching for new incoming events.  This
 * function starts that activity.  We want to have control over when we start watching.
 */
void WebSocket::startReader() {
	ESP_LOGD(LOG_TAG, ">> startReader: Socket: %s", m_socket.toString().c_str());
	m_pWebSockerReader->start(this);
} // startReader
