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
	std::string   m_fileName;
	size_t        m_length;
	WebSocket*    m_pWebSocket;
public:
	WebSocketFileTransfer();
	virtual ~WebSocketFileTransfer();
	void     start(WebSocket *pWebSocket);
};

#endif /* COMPONENTS_CPP_UTILS_WEBSOCKETFILETRANSFER_H_ */
