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
	virtual void onData(std::string data);
	virtual void onError(std::string error);
};

class WebSocket {
private:
	Socket           m_socket;
	WebSocketHandler m_webSocketHandler;
public:
	WebSocket(Socket socket);
	virtual ~WebSocket();
	void close_cpp();
	Socket getSocket();
	void send_cpp(std::string data);
	void setHandler(WebSocketHandler handler);
};

#endif /* COMPONENTS_WEBSOCKET_H_ */
