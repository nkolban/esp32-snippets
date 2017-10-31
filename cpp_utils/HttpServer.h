/*
 * HttpServer.h
 *
 *  Created on: Aug 30, 2017
 *      Author: kolban
 *
 * Implementation of an HTTP server for the ESP32.
 *
 */

#ifndef COMPONENTS_CPP_UTILS_HTTPSERVER_H_
#define COMPONENTS_CPP_UTILS_HTTPSERVER_H_
#include <stdint.h>

#include <vector>
#include "SockServ.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "SockServ.h"
#include <regex>

class HttpServerTask;

/**
 * @brief Handle path matching for an incoming HTTP request.
 */
class PathHandler {
	public:
		PathHandler(
			std::string method,                // The method in the request to be matched.
			std::string pathPattern,           // The pattern in the request to be matched
			void (*pWebServerRequestHandler)   // The handler function to be invoked upon a match.
			(
				HttpRequest*  pHttpRequest,
				HttpResponse* pHttpResponse)
			);
		PathHandler(
			std::string method,                // The method in the request to be matched.
			std::regex* pathPattern,           // The pattern in the request to be matched (regex)
			void (*pWebServerRequestHandler)   // The handler function to be invoked upon a match.
			(
				HttpRequest*  pHttpRequest,
				HttpResponse* pHttpResponse)
			);
		bool match(std::string method, std::string path);   // Does the request method and pattern match?
		void invokePathHandler(HttpRequest* request, HttpResponse* response);
	private:
		std::string m_method;
		std::regex* m_pRegex;
		bool        m_isRegex;
		std::string m_textPattern;
		void (*m_pRequestHandler)(HttpRequest* pHttpRequest, HttpResponse* pHttpResponse);
}; // PathHandler


class HttpServer {
public:
	HttpServer();
	virtual ~HttpServer();

	void        addPathHandler(
		std::string method,
		std::string pathExpr,
		void (*webServerRequestHandler)
		(
			HttpRequest*  pHttpRequest,
			HttpResponse* pHttpResponse)
		);
	void        addPathHandler(
		std::string method,
		std::regex* pRegex,
		void (*webServerRequestHandler)
		(
			HttpRequest*  pHttpRequest,
			HttpResponse* pHttpResponse)
		);
	uint16_t    getPort();       // Get the port on which the Http server is listening.
	std::string getRootPath();   // Get the root of the file system path.
	bool        getSSL();        // Are we using SSL?
	void        setDirectoryListing(bool use);   // Should we list the content of directories?
	void        setRootPath(std::string path);   // Set the root of the file system path.
	void        start(uint16_t portNumber, bool useSSL=false);
	void        stop();          // Stop a previously started server.
private:
	friend class HttpServerTask;
	friend class WebSocket;
	uint16_t                 m_portNumber;         // Port number on which server is listening.
	std::vector<PathHandler> m_pathHandlers;       // Vector of path handlers.
	std::string              m_rootPath;           // Root path into the file system.
	bool                     m_useSSL;             // Is this server listening on an HTTPS port?
	bool                     m_directoryListing;   // Should we list directory content?
	SockServ                 m_sockServ;           // Server socket.
}; // HttpServer

#endif /* COMPONENTS_CPP_UTILS_HTTPSERVER_H_ */
