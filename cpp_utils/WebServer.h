/*
 * WebServer.h
 *
 *  Created on: May 19, 2017
 *      Author: kolban
 */

#ifndef CPP_UTILS_WEBSERVER_H_
#define CPP_UTILS_WEBSERVER_H_
#include <string>
#include <vector>
#include <regex>
#include <map>
#include "sdkconfig.h"
#ifdef CONFIG_MONGOOSE_PRESENT
#include <mongoose.h>



class WebServer;

/**
 * @brief The HTTP request handler definition.
 * When an incoming HTTP request arrives and it matches a handler, this is the function
 * prototype that is invoked.  It is passed a description of the request and a description
 * of the response.
 */

//typedef void (*WebServerRequestHandler)(WebServer::HTTPRequest *pHttpRequest, WebServer::HTTPResponse *pHttpResponse);



/**
 * @brief %WebServer built on Mongoose.
 *
 * A web server.
 */
class WebServer {
public:
	/**
	 * @brief Request wrapper for an HTTP request.
	 */
	class HTTPRequest {
		public:
			HTTPRequest(struct http_message* message);
			std::string getMethod();
			std::string getPath();
			std::map<std::string, std::string> getQuery();
			std::string getBody();
			std::vector<std::string> pathSplit();
		private:
			struct http_message* m_message;
	};
	/**
	 * @brief Response wrapper for an HTTP response.
	 */
	class HTTPResponse {
		public:
			HTTPResponse(struct mg_connection *nc);
			void addHeader(std::string name, std::string value);
			std::string getRootPath();
			void setStatus(int status);
			void setHeaders(std::map<std::string, std::string>  headers);
			void sendData(std::string data);
			void sendData(uint8_t *pData, size_t length);
			void setRootPath(std::string path);
		private:
			struct mg_connection *m_nc;
			std::string m_rootPath;
			int m_status;
			std::map<std::string, std::string> m_headers;
			bool m_dataSent;
	};
	/**
	 * @brief The handler for path matching.
	 *
	 */
	class PathHandler {
		public:
			PathHandler(std::string method, std::string pathPattern,  void (*webServerRequestHandler)(WebServer::HTTPRequest *pHttpRequest, WebServer::HTTPResponse *pHttpResponse));
			bool match(std::string path);
			void invoke(HTTPRequest *request, HTTPResponse *response);
		private:
			std::string m_method;
			std::regex m_pattern;
			void (*m_requestHandler)(WebServer::HTTPRequest *pHttpRequest, WebServer::HTTPResponse *pHttpResponse);
	};
	WebServer();
	virtual ~WebServer();
	void addPathHandler(std::string method, std::string pathExpr, void (*webServerRequestHandler)(WebServer::HTTPRequest *pHttpRequest, WebServer::HTTPResponse *pHttpResponse) );
	std::string getRootPath();
	void setRootPath(std::string path);
	void start(unsigned short port = 80);
	void processRequest(struct mg_connection *mgConnection, struct http_message *message);
private:
	std::string m_rootPath;
	std::vector<PathHandler> m_pathHandlers;

};





#endif // CONFIG_MONGOOSE_PRESENT
#endif /* CPP_UTILS_WEBSERVER_H_ */
