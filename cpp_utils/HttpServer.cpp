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
#include "HttpResponse.h"
#include "WebSocket.h"
static const char* LOG_TAG = "HttpServer";

/**
 * Constructor for HTTP Server
 */
HttpServer::HttpServer() {
	m_portNumber = 80;
}

HttpServer::~HttpServer() {
	ESP_LOGD(LOG_TAG, "~HttpServer");
}

/**
 * @brief Be an HTTP server task.
 */
class HttpServerTask: public Task {
public:
	HttpServerTask(std::string name): Task(name) {};

private:
	HttpServer* m_pHttpServer; // Reference to the HTTP Server

	/**
	 * @brief Process an incoming HTTP Request
	 * We examine each of the path handlers to see if we have a match for the method/path pair.  If we do,
	 * we invoke the handler callback passing in both the request and response.
	 *
	 * If we didn't find a handler, then we are going to behave as a Web Server and try and serve up the
	 * content from the file on the "file system".
	 * @param [in] request The HTTP request to process.
	 */
	void processRequest(HttpRequest &request) {
		ESP_LOGD(LOG_TAG, ">> processRequest: Method: %s, Path: %s",
			request.getMethod().c_str(), request.getPath().c_str())
		for (auto it = m_pHttpServer->m_pathHandlers.begin(); it != m_pHttpServer->m_pathHandlers.end(); ++it) {
			if (it->match(request.getMethod(), request.getPath())) {
				ESP_LOGD(LOG_TAG, "Found a path handler match!!");
				if (request.isWebsocket()) {
					it->invoke(&request, nullptr);
				} else {
					HttpResponse response(&request);

					it->invoke(&request, &response);
				}
				return;
			} // Path handler match
		} // For each path handler

		// Serve up the content from the file on the file system ... if found ...

		ESP_LOGD(LOG_TAG, "No Path handler found");
		HttpResponse response(&request);
		response.setStatus(HttpResponse::HTTP_STATUS_NOT_FOUND, "Not Found");
		response.sendData("");

	} // processRequest


	/**
	 * Perform the task handling for server.
	 */
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
			processRequest(request);
			if (!request.isWebsocket()) {
				request.close_cpp();
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
 * @brief Get the current root path.
 * @return The current root path.
 */
std::string HttpServer::getRootPath() {
	return m_rootPath;
} // getRootPath

/**
 * @brief Set the root path for URL file mapping.
 *
 * When a browser requests a file, it uses the address form:
 *
 * @code{.unparsed}
 * http://<host>:<port>/<path>
 * @endcode
 *
 * The path part can be considered the path to where the file should be retrieved on the
 * file system available to the web server.  Typically, we want a directory structure on the file
 * system to host the web served files and not expose the whole file system.  Using this method
 * we specify the root directory from which the files will be served.
 *
 * @param [in] path The root path on the file system.
 * @return N/A.
 */
void HttpServer::setRootPath(std::string path) {
	m_rootPath = path;
} // setRootPath


/**
 * @brief Start the HTTP server listening.
 * We start an instance of the HTTP server listening.  A new task is spawned to perform this work in the
 * back ground.
 * @param [in] portNumber The port number on which the HTTP server should listen.
 */
void HttpServer::start(uint16_t portNumber) {
	ESP_LOGD(LOG_TAG, ">> start");
	m_portNumber = portNumber;
	HttpServerTask* pHttpServerTask = new HttpServerTask("HttpServerTask");
	pHttpServerTask->start(this);
} // start


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
	m_textPattern    = pathPattern;
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
	ESP_LOGD(LOG_TAG, "match: %s with %s", m_textPattern.c_str(), path.c_str());
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
