/*
 * WebSocket.h
 *
 *  Created on: Sep 2, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_WEBSOCKET_H_
#define COMPONENTS_WEBSOCKET_H_
#include <string>
#include "Socket.h"

class WebSocketHandler {
public:
	virtual void onClose();
	virtual void onMessage(std::string data);
	virtual void onError(std::string error);
};

class WebSocketReader;
class WebSocket {
private:
	friend class WebSocketReader;
	friend class HttpServerTask;
	void              startReader();
	bool              m_receivedClose; // True when we have received a close request.
	bool              m_sentClose;     // True when we have sent a close request.
	Socket            m_socket;        // Partner socket.
	WebSocketHandler  m_webSocketHandler;
	WebSocketReader  *m_pWebSockerReader;


public:
	static const uint16_t CLOSE_NORMAL_CLOSURE        = 1000;
	static const uint16_t CLOSE_GOING_AWAY            = 1001;
	static const uint16_t CLOSE_PROTOCOL_ERROR        = 1002;
	static const uint16_t CLOSE_CANNOT_ACCEPT         = 1003;
	static const uint16_t CLOSE_NO_STATUS_CODE        = 1005;
	static const uint16_t CLOSE_CLOSED_ABNORMALLY     = 1006;
	static const uint16_t CLOSE_NOT_CONSISTENT        = 1007;
	static const uint16_t CLOSE_VIOLATED_POLICY       = 1008;
	static const uint16_t CLOSE_TOO_BIG               = 1009;
	static const uint16_t CLOSE_NO_EXTENSION          = 1010;
	static const uint16_t CLOSE_UNEXPECTED_CONDITION  = 1011;
	static const uint16_t CLOSE_SERVICE_RESTART       = 1012;
	static const uint16_t CLOSE_TRY_AGAIN_LATER       = 1013;
	static const uint16_t CLOSE_TLS_HANDSHAKE_FAILURE = 1015;

	static const uint8_t SEND_TYPE_BINARY = 0x01;
	static const uint8_t SEND_TYPE_TEXT   = 0x02;

	WebSocket(Socket socket);
	virtual ~WebSocket();
	void   close_cpp(uint16_t status=CLOSE_NORMAL_CLOSURE, std::string message = "");
	WebSocketHandler* getHandler();
	Socket getSocket();
	void   send_cpp(std::string data, uint8_t sendType = SEND_TYPE_BINARY);
	void   setHandler(WebSocketHandler handler);
};

#endif /* COMPONENTS_WEBSOCKET_H_ */
