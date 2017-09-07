/*
 * HTTPRequest.h
 *
 *  Created on: Aug 30, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_HTTPREQUEST_H_
#define COMPONENTS_CPP_UTILS_HTTPREQUEST_H_
#include <string>
#include <map>
#include <vector>
#include "Socket.h"
#include "WebSocket.h"
#include "HttpParser.h"



class HttpRequest {
private:
	Socket      m_clientSocket;
	HttpParser  m_parser;
	int         m_status;
	WebSocket  *m_pWebSocket;
public:

	HttpRequest(Socket s);
	virtual ~HttpRequest();
	static const std::string HTTP_HEADER_ACCEPT;
	static const std::string HTTP_HEADER_ALLOW;
	static const std::string HTTP_HEADER_CONNECTION;
	static const std::string HTTP_HEADER_CONTENT_LENGTH;
	static const std::string HTTP_HEADER_CONTENT_TYPE;
	static const std::string HTTP_HEADER_COOKIE;
	static const std::string HTTP_HEADER_HOST;
	static const std::string HTTP_HEADER_LAST_MODIFIED;
	static const std::string HTTP_HEADER_ORIGIN;
	static const std::string HTTP_HEADER_SEC_WEBSOCKET_ACCEPT;
	static const std::string HTTP_HEADER_SEC_WEBSOCKET_PROTOCOL;
	static const std::string HTTP_HEADER_SEC_WEBSOCKET_KEY;
	static const std::string HTTP_HEADER_SEC_WEBSOCKET_VERSION;
	static const std::string HTTP_HEADER_UPGRADE;
	static const std::string HTTP_HEADER_USER_AGENT;

	static const std::string HTTP_METHOD_CONNECT;
	static const std::string HTTP_METHOD_DELETE;
	static const std::string HTTP_METHOD_GET;
	static const std::string HTTP_METHOD_HEAD;
	static const std::string HTTP_METHOD_OPTIONS;
	static const std::string HTTP_METHOD_PATCH;
	static const std::string HTTP_METHOD_POST;
	static const std::string HTTP_METHOD_PUT;



	void close_cpp();
	void dump();
	std::string getBody();
	std::string getHeader(std::string name);
	std::map<std::string, std::string> getHeaders();
	std::string getMethod();
	std::string getPath();
	std::map<std::string, std::string> getQuery();
	Socket      getSocket();
	std::string getVersion();
	WebSocket*  getWebSocket();
	bool        isWebsocket();
	std::vector<std::string> pathSplit();
};

#endif /* COMPONENTS_CPP_UTILS_HTTPREQUEST_H_ */
