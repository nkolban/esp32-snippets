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
#include "Socket.h"
#include "HttpParser.h"



class HttpRequest {
private:
	Socket      m_clientSocket;
	HttpParser  m_parser;
	int         m_status;
	bool        m_isWebsocket;
	std::string m_responseMessage;
	std::string m_responseBody;
	std::map<std::string, std::string> m_responseHeaders;
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

	static const int HTTP_STATUS_CONTINUE;
	static const int HTTP_STATUS_SWITCHING_PROTOCOL;
	static const int HTTP_STATUS_OK;
	static const int HTTP_STATUS_MOVED_PERMANENTLY;
	static const int HTTP_STATUS_BAD_REQUEST;
	static const int HTTP_STATUS_UNAUTHORIZED;
	static const int HTTP_STATUS_FORBIDDEN;
	static const int HTTP_STATUS_NOT_FOUND;
	static const int HTTP_STATUS_METHOD_NOT_ALLOWED;
	static const int HTTP_STATUS_INTERNAL_SERVER_ERROR;
	static const int HTTP_STATUS_NOT_IMPLEMENTED;
	static const int HTTP_STATUS_SERVICE_UNAVAILABLE;

	void addResponseHeader(const std::string name, const std::string value);
	void close_cpp();
	void dump();
	std::string getMethod();
	std::string getPath();
	std::string getRequestBody();
	std::string getRequestHeader(std::string name);
	std::map<std::string, std::string> getRequestHeaders();
	std::string getResponseHeader(std::string name);
	std::map<std::string, std::string> getResponseHeaders();
	Socket getSocket();

	std::string getVersion();
	bool isWebsocket();
	void sendResponse();
	void setResponseBody(const std::string body);
	void setStatus(const int status, const std::string message);
};

#endif /* COMPONENTS_CPP_UTILS_HTTPREQUEST_H_ */
