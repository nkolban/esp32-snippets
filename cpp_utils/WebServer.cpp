/*
 * WebServer.cpp
 *
 *  Created on: May 19, 2017
 *      Author: kolban
 */
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <regex>
#include "sdkconfig.h"
#ifdef CONFIG_MONGOOSE_PRESENT
#define MG_ENABLE_HTTP_STREAMING_MULTIPART 1
#define MG_ENABLE_FILESYSTEM 1
#include "WebServer.h"
#include <esp_log.h>
#include "mongoose.h"
#include <string>
#include <utility>

const char WebServer::HTTPRequest::HTTP_HEADER_ACCEPT[]         = "Accept";
const char WebServer::HTTPRequest::HTTP_HEADER_ALLOW[]          = "Allow";
const char WebServer::HTTPRequest::HTTP_HEADER_CONNECTION[]     = "Connection";
const char WebServer::HTTPRequest::HTTP_HEADER_CONTENT_LENGTH[] = "Content-Length";
const char WebServer::HTTPRequest::HTTP_HEADER_CONTENT_TYPE[]   = "Content-Type";
const char WebServer::HTTPRequest::HTTP_HEADER_CONTENT_ENCODING[]   = "Content-Encoding";
const char WebServer::HTTPRequest::HTTP_HEADER_COOKIE[]         = "Cookie";
const char WebServer::HTTPRequest::HTTP_HEADER_HOST[]           = "Host";
const char WebServer::HTTPRequest::HTTP_HEADER_LAST_MODIFIED[]  = "Last-Modified";
const char WebServer::HTTPRequest::HTTP_HEADER_ORIGIN[]         = "Origin";
const char WebServer::HTTPRequest::HTTP_HEADER_SEC_WEBSOCKET_ACCEPT[]   = "Sec-WebSocket-Accept";
const char WebServer::HTTPRequest::HTTP_HEADER_SEC_WEBSOCKET_PROTOCOL[] = "Sec-WebSocket-Protocol";
const char WebServer::HTTPRequest::HTTP_HEADER_SEC_WEBSOCKET_KEY[]      = "Sec-WebSocket-Key";
const char WebServer::HTTPRequest::HTTP_HEADER_SEC_WEBSOCKET_VERSION[]  = "Sec-WebSocket-Version";
const char WebServer::HTTPRequest::HTTP_HEADER_UPGRADE[]        = "Upgrade";
const char WebServer::HTTPRequest::HTTP_HEADER_USER_AGENT[]     = "User-Agent";

const char WebServer::HTTPRequest::HTTP_METHOD_CONNECT[] = "CONNECT";
const char WebServer::HTTPRequest::HTTP_METHOD_DELETE[]  = "DELETE";
const char WebServer::HTTPRequest::HTTP_METHOD_GET[]     = "GET";
const char WebServer::HTTPRequest::HTTP_METHOD_HEAD[]    = "HEAD";
const char WebServer::HTTPRequest::HTTP_METHOD_OPTIONS[] = "OPTIONS";
const char WebServer::HTTPRequest::HTTP_METHOD_PATCH[]   = "PATCH";
const char WebServer::HTTPRequest::HTTP_METHOD_POST[]    = "POST";
const char WebServer::HTTPRequest::HTTP_METHOD_PUT[]     = "PUT";

const int WebServer::HTTPResponse::HTTP_STATUS_CONTINUE              = 100;
const int WebServer::HTTPResponse::HTTP_STATUS_SWITCHING_PROTOCOL    = 101;
const int WebServer::HTTPResponse::HTTP_STATUS_OK                    = 200;
const int WebServer::HTTPResponse::HTTP_STATUS_MOVED_PERMANENTLY     = 301;
const int WebServer::HTTPResponse::HTTP_STATUS_BAD_REQUEST           = 400;
const int WebServer::HTTPResponse::HTTP_STATUS_UNAUTHORIZED          = 401;
const int WebServer::HTTPResponse::HTTP_STATUS_FORBIDDEN             = 403;
const int WebServer::HTTPResponse::HTTP_STATUS_NOT_FOUND             = 404;
const int WebServer::HTTPResponse::HTTP_STATUS_METHOD_NOT_ALLOWED    = 405;
const int WebServer::HTTPResponse::HTTP_STATUS_INTERNAL_SERVER_ERROR = 500;
const int WebServer::HTTPResponse::HTTP_STATUS_NOT_IMPLEMENTED       = 501;
const int WebServer::HTTPResponse::HTTP_STATUS_SERVICE_UNAVAILABLE   = 503;

#define STATE_NAME 0
#define STATE_VALUE 1

static const char* LOG_TAG = "WebServer";

struct WebServerUserData {
	WebServer* pWebServer;
	WebServer::HTTPMultiPart* pMultiPart;
	WebServer::WebSocketHandler* pWebSocketHandler;
	void* originalUserData;
};


static std::string getContentType(std::string filename)
{
	if (GeneralUtils::endsWith(filename,".htm"))
		return "text/html; charset=UTF-8";
	else if (GeneralUtils::endsWith(filename,".html"))
		return "text/html; charset=UTF-8";
	else if (GeneralUtils::endsWith(filename,".css"))
		return "text/css; charset=UTF-8";
	else if (GeneralUtils::endsWith(filename,".js"))
		return "application/javascript; charset=UTF-8";
	else if (GeneralUtils::endsWith(filename,".png"))
		return "image/png";
	else if (GeneralUtils::endsWith(filename,".gif"))
		return "image/gif";
	else if (GeneralUtils::endsWith(filename,".jpg"))
		return "image/jpeg";
	else if (GeneralUtils::endsWith(filename,".ico"))
		return "image/x-icon";
	else if (GeneralUtils::endsWith(filename,".xml"))
		return "text/xml; charset=UTF-8";
	else if (GeneralUtils::endsWith(filename,".pdf"))
		return "application/x-pdf";
	else if (GeneralUtils::endsWith(filename,".zip"))
		return "application/x-zip";
	else if (GeneralUtils::endsWith(filename,".gz"))
		return "application/x-gzip";
	return "text/plain; charset=UTF-8";
}

/**
 * @brief Convert a Mongoose event type to a string.
 * @param [in] event The received event type.
 * @return The string representation of the event.
 */
static std::string mongoose_eventToString(int event) {
	switch (event) {
		case MG_EV_CONNECT:
			return "MG_EV_CONNECT";
		case MG_EV_ACCEPT:
			return "MG_EV_ACCEPT";
		case MG_EV_CLOSE:
			return "MG_EV_CLOSE";
		case MG_EV_SEND:
			return "MG_EV_SEND";
		case MG_EV_RECV:
			return "MG_EV_RECV";
		case MG_EV_POLL:
			return "MG_EV_POLL";
		case MG_EV_TIMER:
			return "MG_EV_TIMER";
		case MG_EV_HTTP_PART_DATA:
			return "MG_EV_HTTP_PART_DATA";
		case MG_EV_HTTP_MULTIPART_REQUEST:
			return "MG_EV_HTTP_MULTIPART_REQUEST";
		case MG_EV_HTTP_PART_BEGIN:
			return "MG_EV_HTTP_PART_BEGIN";
		case MG_EV_HTTP_PART_END:
			return "MG_EV_HTTP_PART_END";
		case MG_EV_HTTP_MULTIPART_REQUEST_END:
			return "MG_EV_HTTP_MULTIPART_REQUEST_END";
		case MG_EV_HTTP_REQUEST:
			return "MG_EV_HTTP_REQUEST";
		case MG_EV_HTTP_REPLY:
			return "MG_EV_HTTP_REPLY";
		case MG_EV_HTTP_CHUNK:
			return "MG_EV_HTTP_CHUNK";
		case MG_EV_MQTT_CONNACK:
			return "MG_EV_MQTT_CONNACK";
		case MG_EV_MQTT_CONNECT:
			return "MG_EV_MQTT_CONNECT";
		case MG_EV_MQTT_DISCONNECT:
			return "MG_EV_MQTT_DISCONNECT";
		case MG_EV_MQTT_PINGREQ:
			return "MG_EV_MQTT_PINGREQ";
		case MG_EV_MQTT_PINGRESP:
			return "MG_EV_MQTT_PINGRESP";
		case MG_EV_MQTT_PUBACK:
			return "MG_EV_MQTT_PUBACK";
		case MG_EV_MQTT_PUBCOMP:
			return "MG_EV_MQTT_PUBCOMP";
		case MG_EV_MQTT_PUBLISH:
			return "MG_EV_MQTT_PUBLISH";
		case MG_EV_MQTT_PUBREC:
			return "MG_EV_MQTT_PUBREC";
		case MG_EV_MQTT_PUBREL:
			return "MG_EV_MQTT_PUBREL";
		case MG_EV_MQTT_SUBACK:
			return "MG_EV_MQTT_SUBACK";
		case MG_EV_MQTT_SUBSCRIBE:
			return "MG_EV_MQTT_SUBSCRIBE";
		case MG_EV_MQTT_UNSUBACK:
			return "MG_EV_MQTT_UNSUBACK";
		case MG_EV_MQTT_UNSUBSCRIBE:
			return "MG_EV_MQTT_UNSUBSCRIBE";
		case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST:
			return "MG_EV_WEBSOCKET_HANDSHAKE_REQUEST";
		case MG_EV_WEBSOCKET_HANDSHAKE_DONE:
			return "MG_EV_WEBSOCKET_HANDSHAKE_DONE";
		case MG_EV_WEBSOCKET_FRAME:
			return "MG_EV_WEBSOCKET_FRAME";
		case MG_EV_WEBSOCKET_CONTROL_FRAME:
			return "MG_EV_WEBSOCKET_CONTROL_FRAME";
	}
	std::string s;
	s += "Unknown event: ";
	s += event;
	return s;
} //eventToString


static void dumpHttpMessage(struct http_message* pHttpMessage) {
	ESP_LOGD(LOG_TAG, "HTTP Message");
	ESP_LOGD(LOG_TAG, "Message: %.*s", (int) pHttpMessage->message.len, pHttpMessage->message.p);
}

/*
static struct mg_str uploadFileNameHandler(struct mg_connection* mgConnection, struct mg_str fname) {
	ESP_LOGD(LOG_TAG, "uploadFileNameHandler: %s", mgStrToString(fname).c_str());
	return fname;
}
*/


/**
 * @brief Mongoose event handler.
 * The event handler is called when an event occurs associated with the WebServer
 * listening network connection.
 *
 * @param [in] mgConnection The network connection associated with the event.
 * @param [in] event The type of event.
 * @param [in] eventData Data associated with the event.
 * @return N/A.
 */
static void mongoose_event_handler_web_server(struct mg_connection* mgConnection, int event, void* eventData) {
	if (event == MG_EV_POLL) return;
	ESP_LOGD(LOG_TAG, "Event: %s [%d]", mongoose_eventToString(event).c_str(), mgConnection->sock);
	switch (event) {
		case MG_EV_SEND: {
			struct WebServerUserData* pWebServerUserData = (struct WebServerUserData*) mgConnection->user_data;
			WebServer* pWebServer = pWebServerUserData->pWebServer;
			pWebServer->continueConnection(mgConnection);
			break;
		}

		case MG_EV_HTTP_REQUEST: {
			struct http_message* message = (struct http_message*) eventData;
			dumpHttpMessage(message);

			struct WebServerUserData* pWebServerUserData = (struct WebServerUserData*) mgConnection->user_data;
			WebServer* pWebServer = pWebServerUserData->pWebServer;
			pWebServer->processRequest(mgConnection, message);
			break;
		} // MG_EV_HTTP_REQUEST

		case MG_EV_HTTP_MULTIPART_REQUEST: {
			struct http_message* message = (struct http_message*) eventData;
			ESP_LOGD(LOG_TAG, "Multipart Request user data %p", mgConnection->user_data);
//			dumpHttpMessage(message);
			struct WebServerUserData* pWebServerUserData = (struct WebServerUserData*) mgConnection->user_data;
			WebServer* pWebServer = pWebServerUserData->pWebServer;
			pWebServer->processMultiRequest(mgConnection, message);
			break;
		} // MG_EV_HTTP_MULTIPART_REQUEST

		case MG_EV_HTTP_MULTIPART_REQUEST_END: {
			struct WebServerUserData* pWebServerUserData = (struct WebServerUserData*) mgConnection->user_data;
			ESP_LOGD(LOG_TAG, "Multipart Request End orig user data %p user data %p", pWebServerUserData->originalUserData,
					mgConnection->user_data);
			if (pWebServerUserData->pMultiPart != nullptr) {
				// get the status of this upload
				int status = pWebServerUserData->pMultiPart->getStatus();
				delete pWebServerUserData->pMultiPart;
				pWebServerUserData->pMultiPart = nullptr;
				mgConnection->user_data = pWebServerUserData->originalUserData;
				delete pWebServerUserData;
				WebServer::HTTPResponse httpResponse = WebServer::HTTPResponse(mgConnection);
				httpResponse.setStatus(status);
				ESP_LOGI(LOG_TAG, "MultiPart status %d", status);
				if (status == 200)
				{
					httpResponse.sendData("File uploaded OK");
				}
				else
				{
					httpResponse.sendData("File upload FAILED");
				}
			}
			break;
		} // MG_EV_HTTP_MULTIPART_REQUEST_END

		case MG_EV_HTTP_PART_BEGIN: {
			struct WebServerUserData* pWebServerUserData = (struct WebServerUserData*) mgConnection->user_data;
			struct mg_http_multipart_part* part = (struct mg_http_multipart_part*) eventData;
			ESP_LOGD(LOG_TAG, "file_name: \"%s\", var_name: \"%s\", status: %d, user_data: 0x%d",
					part->file_name, part->var_name, part->status, (uint32_t) part->user_data);
			if (pWebServerUserData->pMultiPart != nullptr) {
				pWebServerUserData->pMultiPart->begin(std::string(part->var_name), std::string(part->file_name));
			}
			break;
		} // MG_EV_HTTP_PART_BEGIN

		case MG_EV_HTTP_PART_DATA: {
			struct WebServerUserData *pWebServerUserData = (struct WebServerUserData*) mgConnection->user_data;
			struct mg_http_multipart_part* part = (struct mg_http_multipart_part*) eventData;
			ESP_LOGD(LOG_TAG, "file_name: \"%s\", var_name: \"%s\", status: %d, user_data: 0x%d",
					part->file_name, part->var_name, part->status, (uint32_t) part->user_data);
			if (pWebServerUserData->pMultiPart != nullptr) {
				pWebServerUserData->pMultiPart->data(std::string(part->data.p, part->data.len));
			}
			break;
		} // MG_EV_HTTP_PART_DATA

		case MG_EV_HTTP_PART_END: {
			struct WebServerUserData* pWebServerUserData = (struct WebServerUserData*) mgConnection->user_data;
			struct mg_http_multipart_part* part = (struct mg_http_multipart_part*) eventData;
			ESP_LOGD(LOG_TAG, "file_name: \"%s\", var_name: \"%s\", status: %d, user_data: 0x%d",
					part->file_name, part->var_name, part->status, (uint32_t)part->user_data);
			if (pWebServerUserData->pMultiPart != nullptr) {
				pWebServerUserData->pMultiPart->end();
			}
			break;
		} // MG_EV_HTTP_PART_END

		case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST: {
			struct WebServerUserData* pWebServerUserData = (struct WebServerUserData*) mgConnection->user_data;
			WebServer* pWebServer = pWebServerUserData->pWebServer;
			if (pWebServer->m_pWebSocketHandlerFactory != nullptr) {
				if (pWebServerUserData->pWebSocketHandler != nullptr) {
					ESP_LOGD(LOG_TAG, "Warning: MG_EV_WEBSOCKET_HANDSHAKE_REQUEST: pWebSocketHandler was NOT null");
				}
				struct WebServerUserData* p2 = new WebServerUserData();
				ESP_LOGD(LOG_TAG, "New User_data address 0x%d", (uint32_t) p2);
				p2->originalUserData	= pWebServerUserData;
				p2->pWebServer		  = pWebServerUserData->pWebServer;
				p2->pWebSocketHandler   = pWebServer->m_pWebSocketHandlerFactory->newInstance();
				mgConnection->user_data = p2;
			} else {
				ESP_LOGD(LOG_TAG, "We received a WebSocket request but we have no handler factory!");
			}
			break;
		} // MG_EV_WEBSOCKET_HANDSHAKE_REQUEST

		case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {
			struct WebServerUserData* pWebServerUserData = (struct WebServerUserData*) mgConnection->user_data;
			if (pWebServerUserData->pWebSocketHandler == nullptr) {
				ESP_LOGE(LOG_TAG, "Error: MG_EV_WEBSOCKET_FRAME: pWebSocketHandler is null");
				return;
			}
			pWebServerUserData->pWebSocketHandler->onCreated();
			break;
		} // MG_EV_WEBSOCKET_HANDSHAKE_DONE


		/*
		 * When we receive a MG_EV_WEBSOCKET_FRAME then we have received a chunk of data over the network.
		 * Our goal will be to send this to the web socket handler (if one exists).
		 */
		case MG_EV_WEBSOCKET_FRAME: {
			struct WebServerUserData* pWebServerUserData = (struct WebServerUserData*) mgConnection->user_data;
			if (pWebServerUserData->pWebSocketHandler == nullptr) {
				ESP_LOGE(LOG_TAG, "Error: MG_EV_WEBSOCKET_FRAME: pWebSocketHandler is null");
				return;
			}
			struct websocket_message* ws_message = (websocket_message*) eventData;
			ESP_LOGD(LOG_TAG, "Received data length: %d", ws_message->size);
			pWebServerUserData->pWebSocketHandler->onMessage(std::string((char*) ws_message->data, ws_message->size));
			break;
		} // MG_EV_WEBSOCKET_FRAME

	} // End of switch
} // End of mongoose_event_handler


/**
 * @brief Constructor.
 */
WebServer::WebServer() {
	m_rootPath				 = "";
	m_pMultiPartFactory		= nullptr;
	m_pWebSocketHandlerFactory = nullptr;
} // WebServer


WebServer::~WebServer() {
}


/**
 * @brief Get the current root path.
 * @return The current root path.
 */
const std::string& WebServer::getRootPath() {
	return m_rootPath;
} // getRootPath


/**
 * @brief Register a handler for a path.
 *
 * When a browser request arrives, the request will contain a method (GET, POST, etc) and a path
 * to be accessed.  Using this method we can register a regular expression and, if the incoming method
 * and path match the expression, the corresponding handler will be called.
 *
 * Example:
 * @code{.cpp}
 * static void handle_REST_WiFi(WebServer::HTTPRequest *pRequest, WebServer::HTTPResponse *pResponse) {
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
void WebServer::addPathHandler(const std::string& method, const std::string& pathExpr,
                               void (*handler)(WebServer::HTTPRequest* pHttpRequest,
                                                WebServer::HTTPResponse* pHttpResponse)) {
	m_pathHandlers.push_back(PathHandler(method, pathExpr, handler));
} // addPathHandler


void WebServer::addPathHandler(std::string&& method, const std::string& pathExpr, void (*handler)(WebServer::HTTPRequest* pHttpRequest, WebServer::HTTPResponse* pHttpResponse)) {
	m_pathHandlers.push_back(PathHandler(std::move(method), pathExpr, handler));
} // addPathHandler


/**
 * @brief Run the web server listening at the given port.
 *
 * This function does not return.
 *
 * @param [in] port The port number of which to listen.
 * @return N/A.
 */
void WebServer::start(uint16_t port) {
	ESP_LOGD(LOG_TAG, "WebServer task starting");
	struct mg_mgr mgr;
	mg_mgr_init(&mgr, NULL);

	std::stringstream stringStream;
	stringStream << ':' << port;
	struct mg_connection *mgConnection = mg_bind(&mgr, stringStream.str().c_str(), mongoose_event_handler_web_server);

	if (mgConnection == NULL) {
		ESP_LOGE(LOG_TAG, "No connection from the mg_bind()");
		vTaskDelete(NULL);
		return;
	}

	struct WebServerUserData* pWebServerUserData = new WebServerUserData();
	pWebServerUserData->pWebServer = this;
	pWebServerUserData->pMultiPart = nullptr;
	mgConnection->user_data		= pWebServerUserData; // Save the WebServer instance reference in user_data.
	ESP_LOGD(LOG_TAG, "start: User_data address 0x%d", (uint32_t)pWebServerUserData);
	mg_set_protocol_http_websocket(mgConnection);

	ESP_LOGD(LOG_TAG, "WebServer listening on port %d", port);
	while (true) {
		mg_mgr_poll(&mgr, 2000);
	}
} // run


/**
 * @brief Set the multi part factory.
 * @param [in] pMultiPart A pointer to the multi part factory.
 */
void WebServer::setMultiPartFactory(HTTPMultiPartFactory *pMultiPartFactory) {
	m_pMultiPartFactory = pMultiPartFactory;
}


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
void WebServer::setRootPath(const std::string& path) {
	m_rootPath = path;
} // setRootPath


void WebServer::setRootPath(std::string&& path) {
	m_rootPath = std::move(path);
} // setRootPath


/**
 * @brief Register the factory for creating web socket handlers.
 *
 * @param [in] pWebSocketHandlerFactory The instance that will create WebSocketHandlers.
 * @return N/A.
 */
void WebServer::setWebSocketHandlerFactory(WebSocketHandlerFactory* pWebSocketHandlerFactory) {
	m_pWebSocketHandlerFactory = pWebSocketHandlerFactory;
} // setWebSocketHandlerFactory


/**
 * @brief Constructor.
 * @param [in] nc The network connection for the response.
 */
WebServer::HTTPResponse::HTTPResponse(struct mg_connection* nc) {
	m_nc	   = nc;
	m_status   = 200;
	m_dataSent = false;
} // HTTPResponse


/**
 * @brief Add a header to the response.
 * @param [in] name The name of the header.
 * @param [in] value The value of the header.
 */
void WebServer::HTTPResponse::addHeader(const std::string& name, const std::string& value) {
	m_headers[name] = value;
} // addHeader

void WebServer::HTTPResponse::addHeader(std::string&& name, std::string&& value) {
	m_headers[std::move(name)] = std::move(value);
} // addHeader


/**
 * @brief Build a string representation of the headers.
 * @return A string representation of the headers.
 */
std::string WebServer::HTTPResponse::buildHeaders() {
	std::string headers;
	unsigned long headers_len = 0;

	for (auto iter = m_headers.begin(); iter != m_headers.end(); iter++) {
			if (iter != m_headers.begin()) {
				headers_len += 2;
			}

			headers_len += iter->first.length();
			headers_len += 2;
			headers_len += iter->second.length();
	}
	headers_len += 1;
//	headers.resize(headers_len); // Will not have to resize and recopy during the next loop, we have 2 loops but it still ends up being faster

	for (auto iter = m_headers.begin(); iter != m_headers.end(); iter++) {
			if (iter != m_headers.begin()) {
				headers += "\r\n";
			}

			headers += iter->first;
			headers += ": ";
			headers += iter->second;
	}

	return headers;
} // buildHeaders


/**
 * @brief Send data to the HTTP caller.
 * Send the data to the HTTP caller.  No further data should be sent after this call.
 * @param [in] data The data to be sent to the HTTP caller.
 * @return N/A.
 */
void WebServer::HTTPResponse::sendData(const std::string& data) {
	sendData((uint8_t*) data.data(), data.length());
} // sendData


/**
 * @brief Send data to the HTTP caller.
 * Send the data to the HTTP caller.  No further data should be sent after this call.
 * @param [in] pData The data to be sent to the HTTP caller.
 * @param [in] length The length of the data to be sent.
 * @return N/A.
 */
void WebServer::HTTPResponse::sendData(const uint8_t* pData, size_t length) {
	if (m_dataSent) {
		ESP_LOGE(LOG_TAG, "HTTPResponse: Data already sent!  Attempt to send again/more.");
		return;
	}
	m_dataSent = true;

	mg_send_head(m_nc, m_status, length, buildHeaders().c_str());
	mg_send(m_nc, pData, length);
	m_nc->flags |= MG_F_SEND_AND_CLOSE;
} // sendData


/**
 *
 */
void WebServer::HTTPResponse::sendData(const char* pData, size_t length) {
	sendData((uint8_t*) pData, length);
} // sendData


/**
 *
 */
void WebServer::HTTPResponse::sendChunkHead() {
	if (m_dataSent) {
		ESP_LOGE(LOG_TAG, "HTTPResponse: Chunk headers already sent!  Attempt to send again/more.");
	}
	m_dataSent = true;
	mg_send_head(m_nc, m_status, -1, buildHeaders().c_str());
} // sendChunkHead


/**
 *
 */
void WebServer::HTTPResponse::sendChunk(const char* pData, size_t length) {
	mg_send_http_chunk(m_nc, pData, length);
} // sendChunkHead


/**
 *
 */
void WebServer::HTTPResponse::closeConnection() {
	m_nc->flags |= MG_F_SEND_AND_CLOSE;
} // closeConnection


/**
 * @brief Set the headers to be sent in the HTTP response.
 * @param [in] headers The complete set of headers to send to the caller.
 * @return N/A.
 */
void WebServer::HTTPResponse::setHeaders(const std::map<std::string, std::string>& headers) {
	m_headers = headers;
} // setHeaders


void WebServer::HTTPResponse::setHeaders(std::map<std::string, std::string>&& headers) {
	m_headers = std::move(headers);
} // setHeaders


/**
 * @brief Get the current root path.
 * @return The current root path.
 */
const std::string& WebServer::HTTPResponse::getRootPath() const {
	return m_rootPath;
} // getRootPath


/**
 * @brief Set the root path for URL file mapping.
 * @param [in] path The root path on the file system.
 * @return N/A.
 */
void WebServer::HTTPResponse::setRootPath(const std::string& path) {
	m_rootPath = path;
} // setRootPath


void WebServer::HTTPResponse::setRootPath(std::string&& path) {
	m_rootPath = std::move(path);
} // setRootPath


/**
 * @brief Set the status value in the HTTP response.
 *
 * The default if not set is 200.
 * @param [in] status The HTTP status code sent to the caller.
 * @return N/A.
 */
void WebServer::HTTPResponse::setStatus(int status) {
	m_status = status;
} // setStatus


/**
 * @brief Process an incoming HTTP request.
 *
 * We look at the path of the request and see if it has a matching path handler.  If it does,
 * we invoke the handler function.  If it does not, we try and find a file on the file system
 * that would resolve to the path.
 *
 * @param [in] mgConnection The network connection on which the request was received.
 * @param [in] message The message representing the request.
 */
void WebServer::processRequest(struct mg_connection* mgConnection, struct http_message* message) {
	ESP_LOGD(LOG_TAG, "WebServer::processRequest: Matching: %.*s", (int) message->uri.len, message->uri.p);
	HTTPResponse httpResponse = HTTPResponse(mgConnection);

	// make all rootPaths match
	httpResponse.setRootPath(getRootPath());

	ESP_LOGD(LOG_TAG, "Root = %s", httpResponse.getRootPath().c_str());
	/*
	 * Iterate through each of the path handlers looking for a match with the method and specified path.
	 */
	std::vector<PathHandler>::iterator it;
	for (it = m_pathHandlers.begin(); it != m_pathHandlers.end(); ++it) {
		if ((*it).match(message->method.p, message->method.len, message->uri.p)) {
			HTTPRequest httpRequest(message);
			(*it).invoke(&httpRequest, &httpResponse);
			ESP_LOGD(LOG_TAG, "Found a match!!");
			return;
		}
	} // End of examine path handlers.

	// Because we reached here, it means that we did NOT match a handler.  Now we want to attempt
	// to retrieve the corresponding file content.
	std::string filePath;
	filePath.reserve(httpResponse.getRootPath().length() + message->uri.len + 1);
	filePath += httpResponse.getRootPath();
	filePath.append(message->uri.p, message->uri.len);

	ESP_LOGD(LOG_TAG, "Opening file: %s", filePath.c_str());
	FILE* file = nullptr;

	if (strcmp(filePath.c_str(), "/") != 0) {
		// only try this if it is NOT a .gz file request

		if (!GeneralUtils::endsWith(filePath, "gz")) {
			// if they are willing to handle a gzipped file we should serve one, assuming we
			// have it

			// test to see if they are willing
			HTTPRequest myHttpRequest(message);
			std::string encoding = myHttpRequest.getHeader("Accept-Encoding");

			if (encoding.find("gzip") != std::string::npos) {
				// they seem willing to accept gzipped files, see if we have one
				std::string gzFilePath(filePath + ".gz");
				file = fopen(gzFilePath.c_str(), "rb");

				if (file != nullptr) {
					// add some needed headers
					httpResponse.addHeader("Content-Encoding", "gzip");
				}
			}
		}

		if ( file == nullptr ){
			file = fopen(filePath.c_str(), "rb");
		}
	}

	if (file != nullptr) {
		// we should be setting the type in the response

		httpResponse.addHeader("Content-Type", getContentType(filePath));

		auto pData = (uint8_t*)malloc(MAX_CHUNK_LENGTH);
		size_t read = fread(pData, 1, MAX_CHUNK_LENGTH, file);

		if (read >= MAX_CHUNK_LENGTH) {
			httpResponse.sendChunkHead();
			httpResponse.sendChunk((char*) pData, read);
			fclose(unfinishedConnection[mgConnection->sock]);
			unfinishedConnection[mgConnection->sock] = file;
		} else {
			fclose(file);
			httpResponse.sendData(pData, read);
		}
		free(pData);
	} else {
		// Handle unable to open file
		httpResponse.setStatus(404); // Not found
		httpResponse.sendData("");
	}
} // processRequest

/**
 * @brief Process an incoming HTTP multipart request.
 *
 * We look at the path of the request and see if it has a matching path handler.  If it does,
 * we invoke the proper multipart factory.  If it does not, we fail with 404
 *
 * @param [in] mgConnection The network connection on which the request was received.
 * @param [in] message The message representing the request.
 */
void WebServer::processMultiRequest(struct mg_connection* mgConnection, struct http_message* message) {
	ESP_LOGD(LOG_TAG, "WebServer::processMultiRequest: Matching: %.*s", (int) message->uri.len, message->uri.p);
	HTTPResponse httpResponse = HTTPResponse(mgConnection);

	// make all rootPaths match
	httpResponse.setRootPath(getRootPath());

	ESP_LOGD(LOG_TAG, "Root = %s", httpResponse.getRootPath().c_str());

	// are they looking for a POST method?

	std::string method;
	method.append(message->method.p, message->method.len);

	if (method == "POST" || method == "DELETE")
	{
		std::string matchMethod = method + "_MULTI";

		/*
		 * Iterate through each of the path handlers looking for a match with the method and specified path.
		 */
		std::vector<PathHandler>::iterator it;
		for (it = m_pathHandlers.begin(); it != m_pathHandlers.end(); ++it) {
			if ((*it).match(matchMethod.c_str(), matchMethod.length(), message->uri.p)) {
				ESP_LOGD(LOG_TAG, "Found a match!!");

				struct WebServerUserData *pWebServerUserData = (struct WebServerUserData*) mgConnection->user_data;
				ESP_LOGD(LOG_TAG, "User_data address 0x%d", (uint32_t) pWebServerUserData);
				WebServer* pWebServer = pWebServerUserData->pWebServer;

				if (pWebServer->m_pMultiPartFactory == nullptr)
					break;

				WebServer::HTTPMultiPart* pMultiPart = pWebServer->m_pMultiPartFactory->newInstance();
				std::string uri;
				uri.append(message->uri.p, message->uri.len);
				pMultiPart->setUri(uri);
				pMultiPart->setMethod(method);
				struct WebServerUserData* p2 = new WebServerUserData();

				ESP_LOGD(LOG_TAG, "New User_data address 0x%d", (uint32_t) p2);

				p2->originalUserData	= pWebServerUserData;
				p2->pWebServer		  = pWebServerUserData->pWebServer;
				p2->pMultiPart		  = pMultiPart;
				p2->pWebSocketHandler   = nullptr;
				mgConnection->user_data = p2;
				return;
			}
		} // End of examine path handlers.
	}

	// Handle unable to handle multipart post
	httpResponse.setStatus(404); // Not found
	httpResponse.sendData("No Multipart handler");

} // processMultiRequest

void WebServer::continueConnection(struct mg_connection* mgConnection) {
	if (unfinishedConnection.count(mgConnection->sock) == 0) return;

	HTTPResponse httpResponse = HTTPResponse(mgConnection);

	FILE* file = unfinishedConnection[mgConnection->sock];
	auto pData = (char*) malloc(MAX_CHUNK_LENGTH);
	size_t length = fread(pData, 1, MAX_CHUNK_LENGTH, file);

	httpResponse.sendChunk(pData, length);
	if (length < MAX_CHUNK_LENGTH) {
		fclose(file);
		httpResponse.closeConnection();
		unfinishedConnection.erase(mgConnection->sock);
		httpResponse.sendChunk("", 0);
	}
	free(pData);
}


/**
 * @brief Construct an instance of a PathHandler.
 *
 * @param [in] method The method to be matched.
 * @param [in] pathPattern The path pattern to be matched.
 * @param [in] webServerRequestHandler The request handler to be called.
 */
WebServer::PathHandler::PathHandler(const std::string& method, const std::string& pathPattern, void (*webServerRequestHandler) (WebServer::HTTPRequest* pHttpRequest, WebServer::HTTPResponse* pHttpResponse)) {
	m_method		 = method;
	m_pattern		= std::regex(pathPattern);
	m_requestHandler = webServerRequestHandler;
} // PathHandler

WebServer::PathHandler::PathHandler(std::string&& method, const std::string& pathPattern, void (*webServerRequestHandler) (WebServer::HTTPRequest* pHttpRequest, WebServer::HTTPResponse* pHttpResponse)) {
	m_method		 = std::move(method);
	m_pattern		= std::regex(pathPattern);
	m_requestHandler = webServerRequestHandler;
} // PathHandler


/**
 * @brief Determine if the path matches.
 *
 * @param [in] method The method to be matched.
 * @param [in] method_len The method's length
 * @param [in] path The path to be matched.
 * @return True if the path matches.
 */
bool WebServer::PathHandler::match(const char* method, size_t method_len, const char* path) {
	//ESP_LOGD(LOG_TAG, "match: %s with %s", m_pattern.c_str(), path.c_str());
	if (method_len != m_method.length() || strncmp(method, m_method.c_str(), method_len) != 0) {
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
void WebServer::PathHandler::invoke(WebServer::HTTPRequest* request, WebServer::HTTPResponse* response) {
	m_requestHandler(request, response);
} // invoke


/**
 * @brief Create an HTTPRequest instance.
 * When mongoose received an HTTP request, we want to encapsulate that to hide the
 * mongoose complexities.  We create an instance of this class to hide those.
 * @param [in] message The description of the underlying Mongoose message.
 */
WebServer::HTTPRequest::HTTPRequest(struct http_message* message) {
	m_message = message;

	// loop through the possible headers (there are only 20)

	for (int x = 0; x < MG_MAX_HTTP_HEADERS; x++)
	{
		std::string headerName;
		std::string headerValue;

		if (m_message->header_names[x].len <= 0)
		{
			break; // we are done
		}

		// get a string of the name
		headerName.clear();
		headerName.append(m_message->header_names[x].p, m_message->header_names[x].len);

		// get a string of the value

		headerValue.clear();
		headerValue.append(m_message->header_values[x].p, m_message->header_values[x].len);

		// now add the entry to the map
		m_headers.insert(HeaderPair(headerName, headerValue));
	}
} // HTTPRequest


/**
 * @brief Get the body of the request.
 * When an HTTP request is either PUT or POST then it may contain a payload that is also
 * known as the body.  This method returns that payload (if it exists). Careful, because it's not a standard string
 * that is terminated by a null character, use the getBodyLen() function to determine the body length
 * @return The body of the request.
 */
const char* WebServer::HTTPRequest::getBody() const {
	return m_message->body.p;
} // getBody


/**
 * @brief Get the length of the body of the request.
 * When an HTTP request is either PUT or POST then it may contain a payload that is also
 * known as the body.  This method returns that payload (if it exists).
 * @return The length of the body of the request.
 */
size_t WebServer::HTTPRequest::getBodyLen() const {
	return m_message->body.len;
} // getBodyLen


/**
 * @brief Get the method of the request.
 * An HTTP request contains a request method which is one of GET, PUT, POST, etc.
 * @return The method of the request. Careful, because it's not a standard string
 * that is terminated by a null character, use the getMethodLen() function to determine the method length
 * @return The body of the request.
 */
const char* WebServer::HTTPRequest::getMethod() const {
	return m_message->method.p;
} // getMethod


/**
 * @brief Get the length of the method of the request.
 * An HTTP request contains a request method which is one of GET, PUT, POST, etc.
 * @return The length of the method of the request.
 */
size_t WebServer::HTTPRequest::getMethodLen() const {
	return m_message->method.len;
} // getMethodLen


/**
 * @brief Get the path of the request.
 * The path of an HTTP request is the portion of the URL that follows the hostname/port pair
 * but does not include any query parameters. Careful, because it's not a standard string
 * that is terminated by a null character, use the getPathLen() function to determine the path length
 * @return The body of the request.
 * @return The path of the request.
 */
const char* WebServer::HTTPRequest::getPath() const {
	return m_message->uri.p;
} // getPath


/**
 * @brief Get the path of the request.
 * The path of an HTTP request is the portion of the URL that follows the hostname/port pair
 * but does not include any query parameters.
 * @return The path of the request.
 */
size_t WebServer::HTTPRequest::getPathLen() const {
	return m_message->uri.len;
} // getPath


/**
 * @brief Get the query part of the request.
 * The query is a set of name = value pairs.  The return is a map keyed by the name items.
 *
 * @return The query part of the request.
 */
std::map<std::string, std::string> WebServer::HTTPRequest::getQuery() const {
	// Walk through all the characters in the query string maintaining a simple state machine
	// that lets us know what we are parsing.
	std::map<std::string, std::string> queryMap;
	const char* queryString = m_message->query_string.p;
	size_t queryStringLen = m_message->query_string.len;

	/*
	 * We maintain a simple state machine with states of:
	 * * STATE_NAME - We are parsing a name.
	 * * STATE_VALUE - We are parsing a value.
	 */
	int state = STATE_NAME;
	std::string name = "";
	std::string value;
	// Loop through each character in the query string.
	for (size_t i = 0; i < queryStringLen; i++) {
		char currentChar = queryString[i];
		if (state == STATE_NAME) {
			if (currentChar != '=') {
				name += currentChar;
			} else {
				state = STATE_VALUE;
				value = "";
			}
		} // End state = STATE_NAME
		else { // if (state == STATE_VALUE)
			if (currentChar != '&') {
				value += currentChar;
			} else {
				//ESP_LOGD(LOG_TAG, "name=%s, value=%s", name.c_str(), value.c_str());
				queryMap[name] = value;
				state = STATE_NAME;
				name = "";
			}
		} // End state = STATE_VALUE
	} // End for loop
	if (state == STATE_VALUE) {
		//ESP_LOGD(LOG_TAG, "name=%s, value=%s", name.c_str(), value.c_str());
		queryMap[name] = value;
	}
	return queryMap;
} // getQuery


/**
 * @brief Return the constituent parts of the path.
 * If we imagine a path as composed of parts separated by slashes, then this function
 * returns a vector composed of the parts.  For example:
 *
 * ```
 * /x/y/z
 * ```
 * will break out to:
 *
 * ```
 * path[0] = ""
 * path[1] = "x"
 * path[2] = "y"
 * path[3] = "z"
 * ```
 *
 * @return A vector of the constituent parts of the path.
 */
std::vector<std::string> WebServer::HTTPRequest::pathSplit() const {
	std::istringstream stream(std::string(getPath(), getPathLen())); // I don't know if there's a better istringstream constructor for this
	std::vector<std::string> ret;
	std::string pathPart;
	while (std::getline(stream, pathPart, '/')) {
		ret.push_back(pathPart);
	}
	// Debug
	for (int i = 0; i < ret.size(); i++) {
		ESP_LOGD(LOG_TAG, "part[%d]: %s", i, ret[i].c_str());
	}
	return ret;
} // pathSplit

/**
 * @brief Return a std::map of the headers.
 *
 * @return A map of header strings and their values.
 */

HeaderMap WebServer::HTTPRequest::getHeaders(void) const {
	return m_headers;
} //getHeaders

/**
 * @brief Return a std::map of the headers.
 *
 * @return A map of header strings and their values.
 */
std::string WebServer::HTTPRequest::getHeader(std::string headerName) const {
	// get all the headers in a map

	HeaderMap::const_iterator headerIt = m_headers.find(headerName);

	if (headerIt != m_headers.end())
	{
		return headerIt->second;
	}

	return std::string();
}

/**
 * @brief Parse the request message as a form.
 * @return A map containing the names/values of the form elements that were found.
 */
std::map<std::string, std::string> WebServer::HTTPRequest::parseForm() {
	ESP_LOGD(LOG_TAG, ">> parseForm");
	std::map<std::string, std::string> map;
	// A form is composed of name=value pairs where each pair is separated with an "&" character.
	// Our algorithm is to split all the pairs by "&" and then split all the name/value pairs by "=".
	std::istringstream ss(getBody());         // Get the body of the request.
	std::string currentEntry;
	while (std::getline(ss, currentEntry, '&')) {   // For each form entry.
		ESP_LOGD(LOG_TAG, "Processing: %s", currentEntry.c_str());   // Debug
		std::istringstream currentPair(currentEntry);   // Prepare to parse the name=value string.
		std::string name;                               // Declare the name variable.
		std::string value;                              // Declare the value variable.

		std::getline(currentPair, name, '=');           // Parse the current form entry into name/value.
		currentPair >> value;                           // The value is what remains.
		map[name] = urlDecode(value);                   // Decode the field which may have been encoded.
		ESP_LOGD(LOG_TAG, " %s = \"%s\"", name.c_str(), map[name].c_str());   // Debug
                              // Add the form entry into the map.
	} // Processed all form entries.
	ESP_LOGD(LOG_TAG, "<< parseForm");                // Debug
	return map;                                       // Return the map of form entries.
} // parseForm

/**
 * @brief Decode a URL/form
 * @param [in] str
 * @return The decoded string.
 */
std::string WebServer::HTTPRequest::urlDecode(std::string str) {
	// https://stackoverflow.com/questions/154536/encode-decode-urls-in-c
	std::string ret;
	char ch;
	int ii, len = str.length();

	for (int i = 0; i < len; i++) {
		if (str[i] != '%'){
			if (str[i] == '+') {
				ret += ' ';
			} else {
				ret += str[i];
			}
		} else {
			sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
			ch = static_cast<char>(ii);
			ret += ch;
			i = i + 2;
		}
	}
	return ret;
} // urlDecode

/**
 * @brief Indicate the beginning of a multipart part.
 * An HTTP Multipart form is where each of the fields in the form are broken out into distinct
 * sections.  We commonly see this with file uploads.
 * @param [in] varName The name of the form variable.
 * @param [in] fileName The name of the file being uploaded (may not be present).
 * @return N/A.
 */
void WebServer::HTTPMultiPart::begin(const std::string& varName, const std::string& fileName) {
	ESP_LOGD(LOG_TAG, "WebServer::HTTPMultiPart::begin(varName=\"%s\", fileName=\"%s\")",
		varName.c_str(), fileName.c_str());
} // WebServer::HTTPMultiPart::begin


/**
 * @brief Indicate the end of a multipart part.
 * This will eventually be called after a corresponding begin().
 * @return N/A.
 */
void WebServer::HTTPMultiPart::end() {
	ESP_LOGD(LOG_TAG, "WebServer::HTTPMultiPart::end()");
} // WebServer::HTTPMultiPart::end


/**
 * @brief Indicate the arrival of data of a multipart part.
 * This will be called after a begin() and it may be called many times.  Each
 * call will result in more data.  The end of the data will be indicated by a call to end().
 * @param [in] data The data received in this callback.
 * @return N/A.
 */
void WebServer::HTTPMultiPart::data(const std::string& data) {
	ESP_LOGD(LOG_TAG, "WebServer::HTTPMultiPart::data(), length=%d", data.length());
} // WebServer::HTTPMultiPart::data


/**
 * @brief Indicate the end of all the multipart parts.
 * @return N/A.
 */
void WebServer::HTTPMultiPart::multipartEnd() {
	ESP_LOGD(LOG_TAG, "WebServer::HTTPMultiPart::multipartEnd()");
} // WebServer::HTTPMultiPart::multipartEnd


/**
 * @brief Indicate the start of all the multipart parts.
 * @return N/A.
 */
void WebServer::HTTPMultiPart::multipartStart() {
	ESP_LOGD(LOG_TAG, "WebServer::HTTPMultiPart::multipartStart()");
} // WebServer::HTTPMultiPart::multipartStart

void WebServer::HTTPMultiPart::setUri(std::string uri) {
	ESP_LOGD(LOG_TAG, "WebServer::HTTPMultiPart::setUri(%s)", uri.c_str());
} // WebServer::HTTPMultiPart::setUri

std::string WebServer::HTTPMultiPart::getUri() {
	ESP_LOGD(LOG_TAG, "WebServer::HTTPMultiPart::getUri()");
	return "UNKNOWN";
} // WebServer::HTTPMultiPart::geturi

void WebServer::HTTPMultiPart::setMethod(std::string method) {
	ESP_LOGD(LOG_TAG, "WebServer::HTTPMultiPart::setMethod(%s0)", method.c_str());
} // WebServer::HTTPMultiPart::setMethod

std::string WebServer::HTTPMultiPart::getMethod() {
	ESP_LOGD(LOG_TAG, "WebServer::HTTPMultiPart::getMethod()");
	return "UNKNOWN";
} // WebServer::HTTPMultiPart::getMethod

int WebServer::HTTPMultiPart::getStatus() {
	ESP_LOGD(LOG_TAG, "WebServer::HTTPMultiPart::getStatus()");
	return 404;
} // WebServer::HTTPMultiPart::getStatus

class MyMultiPart : public WebServer::HTTPMultiPart {
public:
	void begin(std::string varName,	std::string fileName) {
		ESP_LOGD(LOG_TAG, "MyMultiPart begin(): varName=%s, fileName=%s",
			varName.c_str(), fileName.c_str());
		m_currentVar = varName;
		if (varName == "path") {
			m_path = "";
		} else if (varName == "myfile") {
			m_fileData = "";
			m_fileName = fileName;
		}
	} // begin

	void end() {
		ESP_LOGD(LOG_TAG, "MyMultiPart end()");
		if (m_currentVar == "myfile") {
			std::string fileName = m_path + "/" + m_fileName;
			ESP_LOGD(LOG_TAG, "Write to file: %s ... data: %s", fileName.c_str(), m_fileData.c_str());

			//std::ofstream myfile;
			//myfile.open(fileName, std::ios::out | std::ios::binary | std::ios::trunc);
			//myfile << m_fileData;
			//myfile.close();

			FILE *ffile = fopen(fileName.c_str(), "w");
			fwrite(m_fileData.data(), m_fileData.length(), 1, ffile);
			fclose(ffile);
		}
	} // end

	void data(std::string data) {
		ESP_LOGD(LOG_TAG, "MyMultiPart data(): length=%d", data.length());
		if (m_currentVar == "path") {
			m_path += data;
		}
		else if (m_currentVar == "myfile") {
			m_fileData += data;
		}
	} // data

	void multipartEnd() {
		ESP_LOGD(LOG_TAG, "MyMultiPart multipartEnd()");
	} // multipartEnd

	void multipartStart() {
		ESP_LOGD(LOG_TAG, "MyMultiPart multipartStart()");
	} // multipartStart

	void setUri(std::string uri) {
		m_uri = uri;
	}

	std::string getUri(void) {
		return m_uri;
	}

	void setMethod(std::string method) {
		m_method = method;
	}

	std::string getMethod(void) {
		return m_method;
	}

	int getStatus(void) {
		return m_status;
	}

private:
	std::string m_fileName;
	std::string m_path;
	std::string m_currentVar;
	std::string m_fileData;
	std::string m_method;
	std::string m_uri;
	int m_status;
};

class MyMultiPartFactory : public WebServer::HTTPMultiPartFactory {
	WebServer::HTTPMultiPart* newInstance() {
		return new MyMultiPart();
	}
};

/**
 * @brief Indicate that a new WebSocket instance has been created.
 * @return N/A.
 */
void WebServer::WebSocketHandler::onCreated() {
} // onCreated


/**
 * @brief Indicate that a new message has been received.
 * @param [in] message The message received from the client.
 * @return N/A.
 */
void WebServer::WebSocketHandler::onMessage(const std::string& message){
} // onMessage

/**
 * @brief Indicate that the client has closed the WebSocket.
 * @return N/A
 */
void WebServer::WebSocketHandler::onClosed() {
} // onClosed


/**
 * @brief Send data down the WebSocket
 * @param [in] message The message to send down the socket.
 * @return N/A.
 */
void WebServer::WebSocketHandler::sendData(const std::string& message) {
	ESP_LOGD(LOG_TAG, "WebSocketHandler::sendData(length=%d)", message.length());
	mg_send_websocket_frame(m_mgConnection,
	   WEBSOCKET_OP_BINARY | WEBSOCKET_OP_CONTINUE,
	   message.data(), message.length());
} // sendData


/**
 * @brief Send data down the WebSocket
 * @param [in] data The message to send down the socket.
 * @param [in] size The size of the message
 * @return N/A.
 */
void WebServer::WebSocketHandler::sendData(const uint8_t* data, uint32_t size) {
	mg_send_websocket_frame(m_mgConnection,
	   WEBSOCKET_OP_BINARY | WEBSOCKET_OP_CONTINUE,
	   data, size);
} // sendData


/**
 * @brief Close the WebSocket from the web server end.
 * Previously a client has connected to us and created a WebSocket.  By making this call we are
 * declaring that the socket should be closed from the server end.
 * @return N/A.
 */
void WebServer::WebSocketHandler::close() {
	mg_send_websocket_frame(m_mgConnection, WEBSOCKET_OP_CLOSE, nullptr, 0);
} // close

#endif // CONFIG_MONGOOSE_PRESENT
