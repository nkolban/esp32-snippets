/*
 * HttpServer.cpp
 * Design:
 * This class represents an HTTP server.  We create an instance of the class and then it is configured.
 * When the user has completed the configuration, they execute the start() method which starts it running.
 * A subsequent call to stop() will stop it.  When start() is called, a new task is created which listens
 * for incoming messages.
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
#include "FileSystem.h"
#include "WebSocket.h"
#include "GeneralUtils.h"
#include "Memory.h"
static const char* LOG_TAG = "HttpServer";

#undef close


/**
 * Constructor for HTTP Server
 */
HttpServer::HttpServer() {
	m_portNumber = 80;            // The default port number.
	m_clientTimeout = 5;            // The default timeout 5 seconds.
	m_rootPath   = "";            // The default path.
	m_useSSL     = false;         // Default SSL is no.
	setDirectoryListing(false);   // Default directory listing is disabled.
	m_fileBufferSize = 4 * 1024;	// Default size of the file buffer.
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
	HttpServerTask(std::string name): Task(name, 16 * 1024) {
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
	 *
	 * @param [in] request The HTTP request to process.
	 */
	void processRequest(HttpRequest& request) {
		ESP_LOGD("HttpServerTask", ">> processRequest: Method: %s, Path: %s",
			request.getMethod().c_str(), request.getPath().c_str());

		// Loop over all the path handlers we have looking for the first one that matches.  Note that none of them
		// need to match.  If we find one that does, then invoke the handler and that is the end of processing.
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

		std::string fileName = m_pHttpServer->getRootPath() + request.getPath(); // Build the absolute file name to read.

		// If the file name ends with a '/' then remove it ... we are normalizing to NO trailing slashes.
		if (GeneralUtils::endsWith(fileName, '/')) {
			fileName = fileName.substr(0, fileName.length() - 1);
		}

		HttpResponse response(&request);
		// Test if the path is a directory.
		if (FileSystem::isDirectory(fileName)) {
			ESP_LOGD(LOG_TAG, "Path %s is a directory", fileName.c_str());
			m_pHttpServer->listDirectory(fileName, response);   // List the contents of the directory.
			return;
		} // Path was a directory.

		response.sendFile(fileName, m_pHttpServer->getFileBufferSize());
	} // processRequest


	/**
	 * @brief Perform the task handling for server.
	 * We loop forever waiting for new client connections to arrive.  When they do, we parse the
	 * content and look for a handler for that content.
	 * @param [in] data A reference to the HttpServer.
	 */
	void run(void* data) {
		m_pHttpServer = (HttpServer*) data;			 // The passed in data is an instance of an HttpServer.
		m_pHttpServer->m_socket.setSSL(m_pHttpServer->m_useSSL);
		m_pHttpServer->m_socket.listen(m_pHttpServer->m_portNumber, false /* is datagram */, true /* Allow address reuse */);
		ESP_LOGD("HttpServerTask", "Listening on port %d", m_pHttpServer->getPort());
		Socket clientSocket;
		while (true) {   // Loop forever.
			ESP_LOGD("HttpServerTask", "Waiting for new peer client");

			try {
				clientSocket = m_pHttpServer->m_socket.accept();   // Block waiting for a new external client connection.
				clientSocket.setTimeout(m_pHttpServer->getClientTimeout());
			} catch (std::exception& e) {
				ESP_LOGE("HttpServerTask", "Caught an exception waiting for new client!");
				m_pHttpServer->m_semaphoreServerStarted.give();  // Release the semaphore .. we are now no longer running.
				return;
			}

			ESP_LOGD("HttpServerTask", "HttpServer that was listening on port %d has received a new client connection; sockFd=%d", m_pHttpServer->getPort(), clientSocket.getFD());

			HttpRequest request(clientSocket);   // Build the HTTP Request from the socket.
			if (request.isWebsocket()) {        // If this is a WebSocket
				clientSocket.setTimeout(0);     //   Clear the timeout.
			}
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
 *	...
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
		std::regex* pathExpr,
		void (*handler)(HttpRequest* pHttpRequest, HttpResponse* pHttpResponse)) {

	// We are maintaining a C++ vector of PathHandler objects.  We add a new entry into that vector.
	m_pathHandlers.push_back(PathHandler(method, pathExpr, handler));
} // addPathHandler


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
 *	...
 * }
 *
 * webServer.addPathHandler("GET", "/ESP32/WiFi", handle_REST_WiFi);
 * @endcode
 *
 * @param [in] method The method being used for access ("GET", "POST" etc).
 * @param [in] path The plain path being accessed.
 * @param [in] handler The callback function to be invoked when a request arrives.
 */
void HttpServer::addPathHandler(
		std::string method,
		std::string path,
		void (*handler)(HttpRequest* pHttpRequest, HttpResponse* pHttpResponse)) {

	// We are maintaining a C++ vector of PathHandler objects.  We add a new entry into that vector.
	m_pathHandlers.push_back(PathHandler(method, path, handler));
} // addPathHandler


/**
 * @brief Get the size of the file buffer.
 * When serving up a file from the file system, we can't afford to read the whole file into RAM before
 * sending it.  As such, we must read the file in chunks.  The buffer size is the size of a chunk to be
 * transmitted before the next chunk is read.
 * @return The file buffer size.
 */
size_t HttpServer::getFileBufferSize() {
	return m_fileBufferSize;
} // getFileBufferSize


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
 * @brief Return whether or not we are using SSL.
 * @return True if we are using SSL.
 */
bool HttpServer::getSSL() {
	return m_useSSL;
} // getSSL


/**
 * Send a directory listing back to the browser.
 * @param [in] path The path of the directory to list.
 * @param [in] response The response object to use to send data back to the browser.
 */
void HttpServer::listDirectory(std::string path, HttpResponse& response) {
	// If path ends with a "/" then remove it.
	if (GeneralUtils::endsWith(path, '/')) {
		path = path.substr(0, path.length()-1);
	}

	// The url path may not be the same as path which is the path on the file system.
	// They will differ if we are using a root path.  We check to see if the file system path
	// starts with the root path, if it does, we remove the root path from the url path.
	std::string urlPath = path;
	if (urlPath.compare(0, getRootPath().length(), getRootPath()) == 0) {
		urlPath = urlPath.substr(getRootPath().length());
	}

	response.addHeader("Content-Type", "text/html");
	response.setStatus(HttpResponse::HTTP_STATUS_OK, "OK");
	response.sendData("<html><head>");
	if (!GeneralUtils::endsWith(path, '/')) {
		response.sendData("<base href='" + urlPath + "/' />");
	}
	response.sendData("</head><body>");
	response.sendData("<h1>" + path + "</h1>");
	response.sendData("<hr/>");
	response.sendData("<p><a href='..'>[To Parent Directory]</a></p>");
	response.sendData("<table style='font-family: monospace;'>");
	auto files = FileSystem::getDirectoryContents(path);
	for (auto it = files.begin(); it != files.end(); ++it) {
		std::stringstream ss;
		ss << "<tr><td><a href='" << it->getName() << "'>" << it->getName() << "</a></td>";
		if (it->isDirectory()) {
			ss << "<td>&lt;dir&gt;</td>";
		} else {
			ss << "<td>" << it->length() << "</td>";
		}

		ss << "</tr>";
		response.sendData(ss.str());
		ESP_LOGD(LOG_TAG, "file: %s", ss.str().c_str());
	}
	response.sendData("</table>");
	response.sendData("<hr/>");
	response.sendData("</body></html>");
	response.close();
} // listDirectory


/**
 * @brief Set different socket timeout for new connections.
 * @param [in] use Set to true to enable directory listing.
 */
void HttpServer::setClientTimeout(uint32_t timeout) {
	m_clientTimeout = timeout;
}


/**
 * @brief Get current socket's timeout for new connections.
 * @param [in] use Set to true to enable directory listing.
 */
uint32_t HttpServer::getClientTimeout() {
	return m_clientTimeout;
}


/**
 * @brief Set whether or not we will list directories.
 * @param [in] use Set to true to enable directory listing.
 */
void HttpServer::setDirectoryListing(bool use) {
	m_directoryListing = use;
} // setDirectoryListening


/**
 * @brief Set the size of the file buffer.
 * When serving up a file from the file system, we can't afford to read the whole file into RAM before
 * sending it.  As such, we must read the file in chunks.  The buffer size is the size of a chunk to be
 * transmitted before the next chunk is read.
 * @param [in] fileBufferSize How large should the file buffer size be?
 */
void HttpServer::setFileBufferSize(size_t fileBufferSize) {
	m_fileBufferSize = fileBufferSize;
} // setFileBufferSize


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
	// Should the user have supplied a path that ends in a "/" remove the trailing slash.  This also
	// means that "/" becomes "".
	if (GeneralUtils::endsWith(path, '/')) {
		path = path.substr(0, path.length()-1);
	}
	m_rootPath = path;
} // setRootPath


/**
 * @brief Start the HTTP server listening.
 * We start an instance of the HTTP server listening.  A new task is spawned to perform this work in the
 * back ground.
 * @param [in] portNumber The port number on which the HTTP server should listen.
 * @param [in] useSSL Should we use SSL?
 */
void HttpServer::start(uint16_t portNumber, bool useSSL) {
	// Design:
	// The start of the HTTP server should be as fast as possible.
	ESP_LOGD(LOG_TAG, ">> start: port: %d, useSSL: %d", portNumber, useSSL);

	// Take the semaphore that says that we are now running.  If we are already running, then end here as
	// there is nothing further to do.
	if (!m_semaphoreServerStarted.take(100, "start")) {
		ESP_LOGD(LOG_TAG, "<< start: Already running");
		return;
	}

	m_useSSL     = useSSL;
	m_portNumber = portNumber;

	HttpServerTask* pHttpServerTask = new HttpServerTask("HttpServerTask");
	pHttpServerTask->start(this);
	ESP_LOGD(LOG_TAG, "<< start");
} // start


/**
 * @brief Shutdown the HTTP server.
 */
void HttpServer::stop() {
	// Shutdown the HTTP Server.  The high level is that we will stop the server socket
	// that is listening for incoming connections.  That will then shutdown all the other
	// activities.
	ESP_LOGD(LOG_TAG, ">> stop");
	m_socket.close();                      // Close the socket that is being used to watch for incoming requests.
	m_semaphoreServerStarted.wait("stop"); // Wait for the server to stop.
	ESP_LOGD(LOG_TAG, "<< stop");
} // stop


/**
 * @brief Construct an instance of a PathHandler.
 *
 * @param [in] method The method to be matched.
 * @param [in] pathPattern The path pattern to be matched.
 * @param [in] webServerRequestHandler The request handler to be called.
 */
PathHandler::PathHandler(std::string method, std::regex* pRegex,
		void (*pWebServerRequestHandler)
		(
			HttpRequest*  pHttpRequest,
			HttpResponse* pHttpResponse)
		) {
	m_method          = method;                  // Save the method we are looking for.
	m_pRegex          = pRegex;                  // Save the Regex
	m_textPattern     = "<Regex>";               // The plain text of the regex pattern.
	m_isRegex         = true;
	m_pRequestHandler = pWebServerRequestHandler; // The handler to be invoked if the pattern matches.
} // PathHandler


/**
 * @brief Construct an instance of a PathHandler.
 *
 * @param [in] method The method to be matched.
 * @param [in] pathPattern The path to be matched.  Must be an exact match.
 * @param [in] webServerRequestHandler The request handler to be called.
 */
PathHandler::PathHandler(std::string method, std::string matchPath,
		void (*pWebServerRequestHandler)
		(
			HttpRequest*  pHttpRequest,
			HttpResponse* pHttpResponse)
		) {
	m_method          = method;                  // Save the method we are looking for.
	m_textPattern     = matchPath;
	m_isRegex         = false;
	m_pRegex          = nullptr;
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
	if (method != m_method) return false;
	if (m_isRegex) {
		ESP_LOGD("PathHandler", "regex matching: %s with %s", m_textPattern.c_str(), path.c_str());
		return std::regex_search(path, *m_pRegex);
	}
	ESP_LOGD("PathHandler", "plain matching: %s with %s", m_textPattern.c_str(), path.c_str());
	return m_textPattern.compare(0, m_textPattern.length(), path) ==0;
} // match


/**
 * @brief Invoke the handler.
 * @param [in] request An object representing the request.
 * @param [in] response An object representing the response.
 * @return N/A.
 */
void PathHandler::invokePathHandler(HttpRequest* request, HttpResponse* response) {
	m_pRequestHandler(request, response);
} // invokePathHandler
