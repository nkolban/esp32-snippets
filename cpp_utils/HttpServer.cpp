/*
 * HttpServer.cpp
 *
 *  Created on: Aug 30, 2017
 *      Author: kolban
 */

#include <fstream>
#include "HttpServer.h"
#include "SockServ.h"
#include "Task.h"
#include <esp_log.h>
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "WebSocket.h"
static const char* LOG_TAG = "HttpServer";

#undef close
/**
 * Constructor for HTTP Server
 */
HttpServer::HttpServer() {
	m_portNumber = 80;         // The default port number.
	m_rootPath = "/";          // The default path.
} // HttpServer

HttpServer::~HttpServer() {
	ESP_LOGD(LOG_TAG, "~HttpServer");
}

/**
 * @brief Be an HTTP server task.
 * Here we define a Task that will be run when the HTTP server starts.  It is this task
 * that executes the majority of the passive work of the server.  It listens for incoming
 * connections and processes them when they arrive.
 */
class HttpServerTask: public Task {
public:
	HttpServerTask(std::string name): Task(name) {
		m_pHttpServer = nullptr;
	};

private:
	HttpServer* m_pHttpServer; // Reference to the HTTP Server

	/**
	 * @brief Process an incoming HTTP Request
	 *
	 * We examine each of the path handlers to see if we have a match for the method/path pair.  If we do,
	 * we invoke the handler callback passing in both the request and response.
	 *
	 * If we didn't find a handler, then we are going to behave as a Web Server and try and serve up the
	 * content from the file on the "file system".
	 * @param [in] request The HTTP request to process.
	 */
	void processRequest(HttpRequest &request) {
		ESP_LOGD("HttpServerTask", ">> processRequest: Method: %s, Path: %s",
			request.getMethod().c_str(), request.getPath().c_str());

		// Loop over all the path handlers we have looking for the first one that matches.  Note that none of them
		// may match.  If we find one that does, then invoke the handler and that is the end of processing.
		for (auto pathHandlerIterartor = m_pHttpServer->m_pathHandlers.begin();
				pathHandlerIterartor != m_pHttpServer->m_pathHandlers.end();
				++pathHandlerIterartor) {
			if (pathHandlerIterartor->match(request.getMethod(), request.getPath())) { // Did we match the handler?
				ESP_LOGD("HttpServerTask", "Found a path handler match!!");
				if (request.isWebsocket()) {                                     // Is this handler to be invoked for a web socket?
					pathHandlerIterartor->invokePathHandler(&request, nullptr);    // Invoke the handler.
					request.getWebSocket()->startReader();
				} else {
					HttpResponse response(&request);
					pathHandlerIterartor->invokePathHandler(&request, &response); // Invoke the handler.
				}
				return;                                                         // End of processing the request
			} // Path handler match
		} // For each path handler

		ESP_LOGD("HttpServerTask", "No Path handler found");
		// If we reach here, then we did not find a handler for the request.


		if (request.isWebsocket()) { 		       // Check to see if we have an un-handled WebSocket
			request.getWebSocket()->close();     // If we do, close the socket as there is nothing further to do.
			return;
		}

		// Serve up the content from the file on the file system ... if found ...
		std::ifstream ifStream;
		std::string fileName = m_pHttpServer->getRootPath() + request.getPath(); // Build the absolute file name to read.
		ESP_LOGD("HttpServerTask", "Opening file: %s", fileName.c_str());
		ifStream.open(fileName, std::ifstream::in | std::ifstream::binary);      // Attempt to open the file for reading.

		// If we failed to open the requested file, then it probably didn't exist so return a not found.
		if (!ifStream.is_open()) {
			HttpResponse response(&request);
			response.setStatus(HttpResponse::HTTP_STATUS_NOT_FOUND, "Not Found");
			response.sendData("");
			return; // Since we failed to open the file, no further work to be done.
		}

		// We now have an open file and want to push the content of that file through to the browser.
		HttpResponse response(&request);
		response.setStatus(HttpResponse::HTTP_STATUS_OK, "OK");
		std::stringstream ss;
		ss << ifStream.rdbuf();
		response.sendData(ss.str());
		ifStream.close();

	} // processRequest


	/**
	 * @brief Perform the task handling for server.
	 * We loop forever waiting for new client connections to arrive.  When they do, we parse the
	 * content and look for a handler for that content.
	 * @param [in] data A reference to the HttpServer.
	 */
	void run(void* data) {
		m_pHttpServer = (HttpServer*)data;           // The passed in data is an instance of an HttpServer.

		SockServ sockServ(m_pHttpServer->getPort()); // Create a socket server on our target port.
		sockServ.start();                            // Start the socket server listening.

		ESP_LOGD("HttpServerTask", "Listening on port %d", m_pHttpServer->getPort());

		while(1) {   // Loop forever.

			ESP_LOGD("HttpServerTask", "Waiting for new peer client");

			Socket clientSocket = sockServ.waitForNewClient();   // Block waiting for a new external client connection.

			ESP_LOGD("HttpServerTask", "HttpServer listening on port %d received a new client connection", m_pHttpServer->getPort());


			HttpRequest request(clientSocket);   // Build the HTTP Request from the socket.
			request.dump();                      // debug.
			processRequest(request);             // Process the request.
			if (!request.isWebsocket()) {        // If this is NOT a WebSocket, then close it as the request
				request.close();                   //   has been completed.
			}
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

	// We are maintaining a C++ vector of PathHandler objects.  We add a new entry into that vector.
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
		void (*pWebServerRequestHandler)
		(
			HttpRequest*  pHttpRequest,
			HttpResponse* pHttpResponse)
		) {
	m_method          = method;                  // Save the method we are looking for.
	m_pattern         = std::regex(pathPattern); // Create the Regex pattern.
	m_textPattern     = pathPattern;             // The plain text of the regex pattern.
	m_pRequestHandler = pWebServerRequestHandler; // The handler to be invoked if the pattern matches.
} // PathHandler


/**
 * @brief Determine if the path matches.
 *
 * @param [in] method The method to be matched.
 * @param [in] path The path to be matched.
 * @return True if the path matches.
 */
bool PathHandler::match(std::string method, std::string path) {
	ESP_LOGD("PathHandler", "matching: %s with %s", m_textPattern.c_str(), path.c_str());
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
void PathHandler::invokePathHandler(HttpRequest* request, HttpResponse *response) {
	m_pRequestHandler(request, response);
} // invokePathHandler
