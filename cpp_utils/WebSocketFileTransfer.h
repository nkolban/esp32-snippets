/*
 * WebSocketFileTransfer.h
 *
 *  Created on: Sep 9, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_WEBSOCKETFILETRANSFER_H_
#define COMPONENTS_CPP_UTILS_WEBSOCKETFILETRANSFER_H_
#include <string>
#include "WebSocket.h"

class WebSocketFileTransfer {
private:
	WebSocket*    m_pWebSocket;   // The WebSocket over which the file data will arrive.
	std::string   m_rootPath;

public:
	WebSocketFileTransfer(std::string rootPath);
	void start(WebSocket* pWebSocket);
};

#endif /* COMPONENTS_CPP_UTILS_WEBSOCKETFILETRANSFER_H_ */
