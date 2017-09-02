/*
 * HttpServer.cpp
 *
 *  Created on: Aug 30, 2017
 *      Author: kolban
 */

#include "HttpServer.h"
#include "SockServ.h"
#include "Task.h"
#include <esp_log.h>
#include "HttpRequest.h"
#include "WebSocket.h"
static const char* LOG_TAG = "HttpServer";

HttpServer::HttpServer() {
	m_portNumber = 0;
}

HttpServer::~HttpServer() {
	ESP_LOGD(LOG_TAG, "~HttpServer");
}

class HttpServerTask: public Task {
public:
	HttpServerTask(std::string name): Task(name) {};
private:
	HttpServer* m_pHttpServer;
	void processRequest(HttpRequest &request) {
		for (auto it = m_pHttpServer->m_pathHandlers.begin(); it != m_pHttpServer->m_pathHandlers.end(); ++it) {
			if ((*it).match(request.getMethod(), request.getPath())) {
				(*it).invoke(&request, &httpResponse);
				ESP_LOGD(LOG_TAG, "Found a match!!");
				return;
			}
		}
	}
	void run(void* data) {
		m_pHttpServer = (HttpServer*)data;
		SockServ sockServ(m_pHttpServer->getPort());
		sockServ.start();
		ESP_LOGD(LOG_TAG, "Listening on port %d", m_pHttpServer->getPort());
		while(1) {
			ESP_LOGD(LOG_TAG, "Waiting for new client");
			Socket clientSocket = sockServ.waitForNewClient();
			HttpRequest request(clientSocket);
			request.dump();
			if (request.isWebsocket()) {
				WebSocket *pWebSocket = new WebSocket(request.getSocket());
			} else {
				processRequest(request);
				/*
				request.setStatus(HttpRequest::HTTP_STATUS_OK, "OK");
				request.addResponseHeader(HttpRequest::HTTP_HEADER_CONTENT_TYPE, "text/plain");
				request.setResponseBody("Hello World");
				request.sendResponse();
				request.close_cpp();
				*/
			}
			ESP_LOGD(LOG_TAG, "Got a new client");
		} // while
	} // run
}; // HttpServerTask

/**
 * @brief Register a handler for a path.
 *
 * When a browser request arrives, the request will contain a method (GET, POST, etc) and a path
 * to be accessed.  Using this method we can register a regular expression and, if the incoming method
 * and path match the expression, the corresponding handler will be called.
 *
 * Example:
 * @code{.cpp}
 * static void handle_REST_WiFi(WebServer::HttpRequest *pRequest, WebServer::HttpResponse *pResponse) {
 *    ...
 * }
 *
 * webServer.addPathHandler("GET", "\\/ESP32\\/WiFi", handle_REST_WiFi);
 * @endcode
 *
 * @param [in] method The method being used for access ("GET", "POST" etc).
 * @param [in] pathExpr The path being accessed.
 * @param [in] handler The callback function to be invoked when a request arrives.
 */
void HttpServer::addPathHandler(
		std::string method,
		std::string pathExpr,
		void (*handler)(HttpRequest *pHttpRequest, HttpResponse *pHttpResponse)) {
	m_pathHandlers.push_back(PathHandler(method, pathExpr, handler));
} // addPathHandler


/**
 * @brief Get the port number on which the HTTP Server is listening.
 * @return The port number on which the HTTP server is listening.
 */
uint16_t HttpServer::getPort() {
	return m_portNumber;
} // getPort


/**
 * @brief Start the HTTP server listening.
 * @param [in] portNumber The port number on which the HTTP server should listen.
 */
void HttpServer::start(uint16_t portNumber) {
	ESP_LOGD(LOG_TAG, ">> start");
	m_portNumber = portNumber;
	HttpServerTask *pHttpServerTask = new HttpServerTask("HttpServerTask");
	pHttpServerTask->start(this);
}


/**
 * @brief Construct an instance of a PathHandler.
 *
 * @param [in] method The method to be matched.
 * @param [in] pathPattern The path pattern to be matched.
 * @param [in] webServerRequestHandler The request handler to be called.
 */
PathHandler::PathHandler(std::string method, std::string pathPattern,
		void (*webServerRequestHandler)(HttpRequest *pHttpRequest, HttpResponse *pHttpResponse)) {
	m_method         = method;
	m_pattern        = std::regex(pathPattern);
	m_requestHandler = webServerRequestHandler;
} // PathHandler


/**
 * @brief Determine if the path matches.
 *
 * @param [in] method The method to be matched.
 * @param [in] path The path to be matched.
 * @return True if the path matches.
 */
bool PathHandler::match(std::string method, std::string path) {
	//ESP_LOGD(tag, "match: %s with %s", m_pattern.c_str(), path.c_str());
	if (method != m_method) {
		return false;
	}
	return std::regex_search(path, m_pattern);
} // match


/**
 * @brief Invoke the handler.
 * @param [in] request An object representing the request.
 * @param [in] response An object representing the response.
 * @return N/A.
 */
void PathHandler::invoke(HttpRequest* request, HttpResponse *response) {
	m_requestHandler(request, response);
} // invoke
