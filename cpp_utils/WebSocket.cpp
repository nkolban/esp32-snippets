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

extern "C" {
	extern uint16_t lwip_ntohs(uint16_t);
	extern uint32_t lwip_ntohl(uint32_t);
	extern uint16_t lwip_htons(uint16_t);
	extern uint32_t lwip_htonl(uint32_t);
}

static const char* LOG_TAG = "WebSocket";

// WebSocket op codes as found in a WebSocket frame.
static const uint8_t OPCODE_CONTINUE = 0x00;
static const uint8_t OPCODE_TEXT     = 0x01;
static const uint8_t OPCODE_BINARY   = 0x02;
static const uint8_t OPCODE_CLOSE    = 0x08;
static const uint8_t OPCODE_PING     = 0x09;
static const uint8_t OPCODE_PONG     = 0x0a;


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
	oss << "Fin: " << (int) frame.fin << ", OpCode: " << (int) frame.opCode;
	switch (frame.opCode) {
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
	oss << ", Mask: " << (int) frame.mask << ", len: " << (int) frame.len;
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
public:
	WebSocketReader() {
		m_end = false;
	}
	void end() {
		m_end = true;
	}

private:
	bool m_end;
	/**
	 * @brief Loop over the web socket waiting for new input.
	 * @param [in] data A pointer to an instance of the WebSocket.
	 */
	void run(void* data) {
		WebSocket* pWebSocket = (WebSocket*) data;
		ESP_LOGD("WebSocketReader", "WebSocketReader Task started, socket: %s", pWebSocket->getSocket().toString().c_str());

		Socket peerSocket = pWebSocket->getSocket();

		Frame frame;
		while (true) {
			if (m_end) break;
			ESP_LOGD("WebSocketReader", "Waiting on socket data for socket %s", peerSocket.toString().c_str());
			int length = peerSocket.receive((uint8_t*) &frame, sizeof(frame), true); // Read exact
			if (length != sizeof(frame)) {
				ESP_LOGD("WebSocketReader", "Socket read error");
				pWebSocket->close();
				return;
			}
			ESP_LOGD("WebSocketReader", "Received data from web socket.  Length: %d", length);
			dumpFrame(frame);

			// The following section parses the WebSocket frame.
			uint32_t payloadLen = 0;
			uint8_t  mask[4];
			if (frame.len < 126) {
				payloadLen = frame.len;
			} else if (frame.len == 126) {
				uint16_t tempLen;
				peerSocket.receive((uint8_t*) &tempLen, sizeof(tempLen), true);
				payloadLen = ntohs(tempLen);
			} else if (frame.len == 127) {
				uint64_t tempLen;
				peerSocket.receive((uint8_t*) &tempLen, sizeof(tempLen), true);
				payloadLen = ntohl((uint32_t) tempLen);
			}
			if (frame.mask == 1) {
				peerSocket.receive(mask, sizeof(mask), true);
			}

			if (payloadLen == 0) {
				ESP_LOGD("WebSocketReader", "Web socket payload is not present");
			} else {
				ESP_LOGD("WebSocketReader", "Web socket payload, length=%d:", payloadLen);
			}

			WebSocketHandler* pWebSocketHandler = pWebSocket->getHandler();
			switch (frame.opCode) {
				case OPCODE_TEXT:
				case OPCODE_BINARY: {
					if (pWebSocketHandler != nullptr) {
						WebSocketInputStreambuf streambuf(pWebSocket->getSocket(), payloadLen, (frame.mask == 1) ? mask : nullptr);
						pWebSocketHandler->onMessage(&streambuf, pWebSocket);
						//streambuf.discard();
					}
					break;
				}

				// If the WebSocket operation code is close then we are closing the connection.
				case OPCODE_CLOSE: {
					pWebSocket->m_receivedClose = true;
					if (pWebSocketHandler != nullptr) { // If we have a handler, invoke the onClose method upon it.
						pWebSocketHandler->onClose();
					}
					pWebSocket->close();                // Close the websocket.
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

				default: {
					ESP_LOGD("WebSocketReader", "Unknown opcode: %d", frame.opCode);
					break;
				}
			} // Switch opCode

		} // while (true)
		ESP_LOGD("WebSocketReader", "<< run");
	} // run
}; // WebSocketReader


/**
 * @brief The default onClose handler.
 * If no over-riding handler is provided for the "close" event, this method is called.
 */
void WebSocketHandler::onClose() {
	ESP_LOGD("WebSocketHandler", ">> onClose");
	ESP_LOGD("WebSocketHandler", "<< onClose");
} // onClose


/**
 * @brief The default onData handler.
 * If no over-riding handler is provided for the "message" event, this method is called.
 * A particularly useful pattern for using onMessage is:
 * ```
 * std::stringstream buffer;
 * buffer << pWebSocketInputRecordStreambuf;
 * ```
 * This will read the whole message into the string stream.
 */
void WebSocketHandler::onMessage(WebSocketInputStreambuf* pWebSocketInputStreambuf, WebSocket* pWebSocket) {
	ESP_LOGD("WebSocketHandler", ">> onMessage");
	ESP_LOGD("WebSocketHandler", "<< onMessage");
} // onData


/**
 * @brief The default onError handler.
 * If no over-riding handler is provided for the "error" event, this method is called.
 */
void WebSocketHandler::onError(std::string error) {
	ESP_LOGD("WebSocketHandler", ">> onError: %s", error.c_str());
	ESP_LOGD("WebSocketHandler", "<< onError");
} // onError


/**
 * @brief Construct a WebSocket instance.
 */
WebSocket::WebSocket(Socket socket) {
	m_receivedClose     = false;
	m_sentClose         = false;
	m_socket            = socket;
	m_pWebSockerReader  = new WebSocketReader();
	m_pWebSocketHandler = nullptr;
} // WebSocket


/**
 * @brief Destructor.
 */
WebSocket::~WebSocket() {
	m_pWebSockerReader->stop();
	delete m_pWebSockerReader;
} // ~WebSocket


/**
 * @brief Close the Web socket
 * @param [in] status The code passed in the close request.
 * @param [in] message A clarification message on the close request.
 */
void WebSocket::close(uint16_t status, std::string message) {
	ESP_LOGD(LOG_TAG, ">> close(): status: %d, message: %s", status, message.c_str());

	if (m_sentClose) {             // If we have previously sent a close request then we can close the underlying socket.
		ESP_LOGD(LOG_TAG, "Closing the underlying socket");
		m_socket.close();            // Close the underlying socket.
		m_pWebSockerReader->end();   // Stop the web socket reader.
		return;
	}
	m_sentClose = true;              // Flag that we have sent a close request.

	Frame frame;                     // Build the web socket frame indicating a close request.
	frame.fin    = 1;
	frame.rsv1   = 0;
	frame.rsv2   = 0;
	frame.rsv3   = 0;
	frame.opCode = OPCODE_CLOSE;
	frame.mask   = 0;
	frame.len    = message.length() + 2;
	int rc = m_socket.send((uint8_t*) &frame, sizeof(frame));

	if (rc > 0) {
		rc = m_socket.send(status);
	}

	if (rc > 0) {
		m_socket.send(message);
	}

	if (m_receivedClose || rc == 0 || rc == -1) {
		m_socket.close();            // Close the underlying socket.
		m_pWebSockerReader->end();   // Stop the web socket reader.
	}
} // close


/**
 * @brief Get the current WebSocketHandler
 * A web socket handler is a user registered class instance that is called when an incoming
 * event received over the network needs to be handled by user code.
 */
WebSocketHandler* WebSocket::getHandler() {
	return m_pWebSocketHandler;
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
void WebSocket::send(std::string data, uint8_t sendType) {
	ESP_LOGD(LOG_TAG, ">> send: Length: %d", data.length());
	Frame frame;
	frame.fin    = 1;
	frame.rsv1   = 0;
	frame.rsv2   = 0;
	frame.rsv3   = 0;
	frame.opCode = (sendType == SEND_TYPE_TEXT) ? OPCODE_TEXT : OPCODE_BINARY;
	frame.mask   = 0;
	if (data.length() < 126) {
		frame.len = data.length();
		m_socket.send((uint8_t*) &frame, sizeof(frame));
	} else {
		frame.len = 126;
		m_socket.send((uint8_t*) &frame, sizeof(frame));
		m_socket.send(htons((uint16_t) data.length()));  // Convert to network byte order from host byte order
	}
	m_socket.send((uint8_t*)data.data(), data.length());
	ESP_LOGD(LOG_TAG, "<< send");
} // send_cpp


/**
 * @brief Send data down the web socket
 * See the WebSocket spec (RFC6455) section "6.1 Sending Data".
 * We build a WebSocket frame, send the frame followed by the data.
 * @param [in] data The data to send down the WebSocket.
 * @param [in] sendType The type of payload.  Either SEND_TYPE_TEXT or SEND_TYPE_BINARY.
 */
void WebSocket::send(uint8_t* data, uint16_t length, uint8_t sendType) {
	ESP_LOGD(LOG_TAG, ">> send: Length: %d", length);
	Frame frame;
	frame.fin    = 1;
	frame.rsv1   = 0;
	frame.rsv2   = 0;
	frame.rsv3   = 0;
	frame.opCode = (sendType==SEND_TYPE_TEXT) ? OPCODE_TEXT : OPCODE_BINARY;
	frame.mask   = 0;
	if (length < 126) {
		frame.len = length;
		m_socket.send((uint8_t*) &frame, sizeof(frame));
	} else {
		frame.len = 126;
		m_socket.send((uint8_t*) &frame, sizeof(frame));
		m_socket.send(htons(length));  // Convert to network byte order from host byte order
	}
	m_socket.send(data, length);
	ESP_LOGD(LOG_TAG, "<< send");
}


/**
 * @brief Set the Web socket handler associated with this Websocket.
 *
 * This will be the user supplied code that will be invoked to process incoming WebSocket
 * events.  An instance of WebSocketHandler is passed in.
 *
 */
void WebSocket::setHandler(WebSocketHandler* pHandler) {
	m_pWebSocketHandler = pHandler;
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


/**
 * @brief Create a Web Socket input record streambuf
 * @param [in] socket The socket we will be reading from.
 * @param [in] dataLength The size of a record.
 * @param [in] bufferSize The size of the buffer we wish to allocate to hold data.
 */
WebSocketInputStreambuf::WebSocketInputStreambuf(
	Socket   socket,
	size_t   dataLength,
	uint8_t* pMask,
	size_t   bufferSize) {
	m_socket     = socket;    // The socket we will be reading from
	m_dataLength = dataLength; // The size of the record we wish to read.
	m_pMask      = pMask;
	m_bufferSize = bufferSize; // The size of the buffer used to hold data
	m_sizeRead   = 0;          // The size of data read from the socket
	m_buffer = new char[bufferSize]; // Create the buffer used to hold the data read from the socket.

	setg(m_buffer, m_buffer, m_buffer); // Set the initial get buffer pointers to no data.
} // WebSocketInputStreambuf


/**
 * @brief Destructor
 */
WebSocketInputStreambuf::~WebSocketInputStreambuf() {
	delete[] m_buffer;
	discard();
} // ~WebSocketInputRecordStreambuf


/**
 * @brief Discard data for the record that has not yet been read.
 *
 * We are working on a logical fixed length record in a socket stream.  This means that we know in advance
 * how big the record should be.  If we have read some data from the stream and no longer wish to consume
 * any further, we have to discard the remaining bytes in the stream before we can get to process the
 * next record.  This function discards the remainder of the data.
 *
 * For example, if our record size is 1000 bytes and we have read 700 bytes and determine that we no
 * longer need to continue, we can't just stop.  There are still 300 bytes in the socket stream that
 * need to be consumed/discarded before we can move on to the next record.
 */
void WebSocketInputStreambuf::discard() {
	uint8_t byte;
	ESP_LOGD("WebSocketInputStreambuf", ">> discard: Discarding %d bytes", m_dataLength - m_sizeRead);
	while(m_sizeRead < m_dataLength) {
		m_socket.receive(&byte, 1);
		m_sizeRead++;
	}
	ESP_LOGD("WebSocketInputStreambuf", "<< discard");
} // discard


/**
 * @brief Get the size of the expected record.
 * @return The size of the expected record.
 */
size_t WebSocketInputStreambuf::getRecordSize() {
	return m_dataLength;
} // getRecordSize


/**
 * @brief Handle the request to read data from the stream but we need more data from the source.
 *
 */
WebSocketInputStreambuf::int_type WebSocketInputStreambuf::underflow() {
	ESP_LOGD("WebSocketInputStreambuf", ">> underflow");

	// If we have already read as many bytes as our record definition says we should read
	// then don't attempt to ready any further.
	if (m_sizeRead >= getRecordSize()) {
		ESP_LOGD("WebSocketInputStreambuf", "<< underflow: Already read maximum");
		return EOF;
	}

	// We wish to refill the buffer.  We want to read data from the socket.  We want to read either
	// the size of the buffer to fill it or the maximum number of bytes remaining to be read.
	// We will choose which ever is smaller as the number of bytes to read into the buffer.
	size_t remainingBytes = getRecordSize() - m_sizeRead;
	size_t sizeToRead;
	if (remainingBytes < m_bufferSize) {
		sizeToRead = remainingBytes;
	} else {
		sizeToRead = m_bufferSize;
	}

	ESP_LOGD("WebSocketInputRecordStreambuf", "- getting next buffer of data; size request: %d", sizeToRead);
	size_t bytesRead = m_socket.receive((uint8_t*) m_buffer, sizeToRead, true);
	if (bytesRead == 0) {
		ESP_LOGD("WebSocketInputRecordStreambuf", "<< underflow: Read 0 bytes");
		return EOF;
	}

	// If the WebSocket frame shows that we have a mask bit set then we have to unmask the data.
	if (m_pMask != nullptr) {
		for (int i = 0; i < bytesRead; i++) {
			m_buffer[i] = m_buffer[i] ^ m_pMask[(m_sizeRead + i) % 4];
		}
	}

	m_sizeRead += bytesRead;  // Increase the count of number of bytes actually read from the source.

	setg(m_buffer, m_buffer, m_buffer + bytesRead); // Changethe buffer pointers to reflect the new data read.
	ESP_LOGD("WebSocketInputRecordStreambuf", "<< underflow - got %d more bytes", bytesRead);
	return traits_type::to_int_type(*gptr());
} // underflow


/**
 * @brief Destructor.
 */
WebSocketHandler::~WebSocketHandler() {
} // ~WebSocketHandler()
