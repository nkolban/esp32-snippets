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

#undef close

class HttpRequest {
public:
	HttpRequest(Socket s);
	virtual ~HttpRequest();
	static const char HTTP_HEADER_ACCEPT[];
	static const char HTTP_HEADER_ALLOW[];
	static const char HTTP_HEADER_CONNECTION[];
	static const char HTTP_HEADER_CONTENT_LENGTH[];
	static const char HTTP_HEADER_CONTENT_TYPE[];
	static const char HTTP_HEADER_COOKIE[];
	static const char HTTP_HEADER_HOST[];
	static const char HTTP_HEADER_LAST_MODIFIED[];
	static const char HTTP_HEADER_ORIGIN[];
	static const char HTTP_HEADER_SEC_WEBSOCKET_ACCEPT[];
	static const char HTTP_HEADER_SEC_WEBSOCKET_PROTOCOL[];
	static const char HTTP_HEADER_SEC_WEBSOCKET_KEY[];
	static const char HTTP_HEADER_SEC_WEBSOCKET_VERSION[];
	static const char HTTP_HEADER_UPGRADE[];
	static const char HTTP_HEADER_USER_AGENT[];

	static const char HTTP_METHOD_CONNECT[];
	static const char HTTP_METHOD_DELETE[];
	static const char HTTP_METHOD_GET[];
	static const char HTTP_METHOD_HEAD[];
	static const char HTTP_METHOD_OPTIONS[];
	static const char HTTP_METHOD_PATCH[];
	static const char HTTP_METHOD_POST[];
	static const char HTTP_METHOD_PUT[];

	void                               close();                      // Close the connection to the client.
	void                               dump();                       // Diagnostic dump of the Http request.
	std::string                        getBody();                    // Get the body of the request.
	std::string                        getHeader(std::string name);  // Get the value of a named header.
	std::map<std::string, std::string> getHeaders();                 // Get all the headers.
	std::string                        getMethod();                  // Get the request method.
	std::string                        getPath();                    // Get the request path.
	std::map<std::string, std::string> getQuery();                   // Get the query part of the request.
	Socket                             getSocket();                  // Get the underlying TCP/IP socket.
	std::string                        getVersion();                 // Get the HTTP version.
	WebSocket*                         getWebSocket();               // Get the WebSocket reference if this is a web socket.
	bool                               isClosed();                   // Has the connection been closed?
	bool                               isWebsocket();                // Is this request to create a web socket?
	std::map<std::string, std::string> parseForm();                  // Parse the body as a form.
	std::vector<std::string>           pathSplit();
	std::string                        urlDecode(std::string str);   // Decode a URL.
private:
	Socket	  m_clientSocket; // The socket connected to the client.
	bool		m_isClosed;	 // Is the client connection closed?
	HttpParser  m_parser;	   // The parse to parse HTTP data.
	WebSocket*  m_pWebSocket;   // A possible reference to a WebSocket object instance.

};

#endif /* COMPONENTS_CPP_UTILS_HTTPREQUEST_H_ */
