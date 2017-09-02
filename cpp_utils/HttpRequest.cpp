/*
 * HTTPRequest.cpp
 *
 *  Created on: Aug 30, 2017
 *      Author: kolban
 */

/*
 * A Websocket hand shake request looks like:
 *
 * GET /chat HTTP/1.1
 * Host: server.example.com
 * Upgrade: websocket
 * Connection: Upgrade
 * Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==
 * Origin: http://example.com
 * Sec-WebSocket-Protocol: chat, superchat
 * Sec-WebSocket-Version: 13
 *
 *
 * A corresponding hand shake response looks like:
 *
 * HTTP/1.1 101 Switching Protocols
 * Upgrade: websocket
 * Connection: Upgrade
 * Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=
 * Sec-WebSocket-Protocol: chat
 *
 * The server key returned in Sec-WebSocket-Accept is the value of Sec-WebSocket-Key passed in the
 * request concatenated with "258EAFA5-E914-47DA-95CA-C5AB0DC85B11" and then take the SHA-1 hash
 * of the result to give a 20 byte value which is then base64() encoded.
 */

#include <sstream>
#include "HttpRequest.h"
#include "GeneralUtils.h"

#include <esp_log.h>
#include <hwcrypto/sha.h>


static const char* LOG_TAG="HttpRequest";

static std::string lineTerminator = "\r\n";

const std::string HttpRequest::HTTP_HEADER_ACCEPT         = "Accept";
const std::string HttpRequest::HTTP_HEADER_ALLOW          = "Allow";
const std::string HttpRequest::HTTP_HEADER_CONNECTION     = "Connection";
const std::string HttpRequest::HTTP_HEADER_CONTENT_LENGTH = "Content-Length";
const std::string HttpRequest::HTTP_HEADER_CONTENT_TYPE   = "Content-Type";
const std::string HttpRequest::HTTP_HEADER_COOKIE         = "Cookie";
const std::string HttpRequest::HTTP_HEADER_HOST           = "Host";
const std::string HttpRequest::HTTP_HEADER_LAST_MODIFIED  = "Last-Modified";
const std::string HttpRequest::HTTP_HEADER_ORIGIN         = "Origin";
const std::string HttpRequest::HTTP_HEADER_SEC_WEBSOCKET_ACCEPT   = "Sec-WebSocket-Accept";
const std::string HttpRequest::HTTP_HEADER_SEC_WEBSOCKET_PROTOCOL = "Sec-WebSocket-Protocol";
const std::string HttpRequest::HTTP_HEADER_SEC_WEBSOCKET_KEY      = "Sec-WebSocket-Key";
const std::string HttpRequest::HTTP_HEADER_SEC_WEBSOCKET_VERSION  = "Sec-WebSocket-Version";
const std::string HttpRequest::HTTP_HEADER_UPGRADE        = "Upgrade";
const std::string HttpRequest::HTTP_HEADER_USER_AGENT     = "User-Agent";

const std::string HttpRequest::HTTP_METHOD_CONNECT = "CONNECT";
const std::string HttpRequest::HTTP_METHOD_DELETE  = "DELETE";
const std::string HttpRequest::HTTP_METHOD_GET     = "GET";
const std::string HttpRequest::HTTP_METHOD_HEAD    = "HEAD";
const std::string HttpRequest::HTTP_METHOD_OPTIONS = "OPTIONS";
const std::string HttpRequest::HTTP_METHOD_PATCH   = "PATCH";
const std::string HttpRequest::HTTP_METHOD_POST    = "POST";
const std::string HttpRequest::HTTP_METHOD_PUT     = "PUT";


const int HttpRequest::HTTP_STATUS_CONTINUE              = 100;
const int HttpRequest::HTTP_STATUS_SWITCHING_PROTOCOL    = 101;
const int HttpRequest::HTTP_STATUS_OK                    = 200;
const int HttpRequest::HTTP_STATUS_MOVED_PERMANENTLY     = 301;
const int HttpRequest::HTTP_STATUS_BAD_REQUEST           = 400;
const int HttpRequest::HTTP_STATUS_UNAUTHORIZED          = 401;
const int HttpRequest::HTTP_STATUS_FORBIDDEN             = 403;
const int HttpRequest::HTTP_STATUS_NOT_FOUND             = 404;
const int HttpRequest::HTTP_STATUS_METHOD_NOT_ALLOWED    = 405;
const int HttpRequest::HTTP_STATUS_INTERNAL_SERVER_ERROR = 500;
const int HttpRequest::HTTP_STATUS_NOT_IMPLEMENTED       = 501;
const int HttpRequest::HTTP_STATUS_SERVICE_UNAVAILABLE   = 503;

std::string buildResponseHash(std::string requestKey) {
	std::string newKey = requestKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	uint8_t shaData[20];
	esp_sha(SHA1, (uint8_t*)newKey.data(), newKey.length(), shaData);
	//GeneralUtils::hexDump(shaData, 20);
	std::string retStr;
	GeneralUtils::base64Encode(std::string((char*)shaData, sizeof(shaData)), &retStr);
	return retStr;
}


HttpRequest::HttpRequest(Socket clientSocket) {
	m_clientSocket = clientSocket;
	m_status       = 0;
	m_isWebsocket  = false;

	m_parser.parse(clientSocket);

	// Is this a Web Socket?
	if (getMethod() == HTTP_METHOD_GET &&
			!getRequestHeader(HTTP_HEADER_HOST).empty() &&
			getRequestHeader(HTTP_HEADER_UPGRADE) == "websocket" &&
			getRequestHeader(HTTP_HEADER_CONNECTION) == "Upgrade" &&
			!getRequestHeader(HTTP_HEADER_SEC_WEBSOCKET_KEY).empty() &&
			!getRequestHeader(HTTP_HEADER_SEC_WEBSOCKET_VERSION).empty()) {
		ESP_LOGD(LOG_TAG, "Websocket detected!");
		m_isWebsocket = true;
		// do something
		// Process the web socket request


		setStatus(HTTP_STATUS_SWITCHING_PROTOCOL, "Switching Protocols");
		addResponseHeader(HTTP_HEADER_UPGRADE, "websocket");
		addResponseHeader(HTTP_HEADER_CONNECTION, "Upgrade");
		addResponseHeader(HTTP_HEADER_SEC_WEBSOCKET_ACCEPT,
			buildResponseHash(getRequestHeader(HTTP_HEADER_SEC_WEBSOCKET_KEY)));
		sendResponse();
	} else {
		ESP_LOGD(LOG_TAG, "Not a Websocket");
	}
} // HttpRequest


HttpRequest::~HttpRequest() {
} // ~HttpRequest


void HttpRequest::addResponseHeader(const std::string name, const std::string value) {
	m_responseHeaders.insert(std::pair<std::string, std::string>(name, value));
} // addResponseHeader


void HttpRequest::close_cpp() {
	m_clientSocket.close_cpp();
} // close_cpp


void HttpRequest::dump() {
	ESP_LOGD(LOG_TAG, "Method: %s, URL: \"%s\", Version: %s", getMethod().c_str(), getPath().c_str(), getVersion().c_str());
	auto headers = getRequestHeaders();
	auto it2 = headers.begin();
	for (; it2 != headers.end(); ++it2) {
		ESP_LOGD(LOG_TAG, "name=\"%s\", value=\"%s\"", it2->first.c_str(), it2->second.c_str());
	}
	ESP_LOGD(LOG_TAG, "Body: \"%s\"", getRequestBody().c_str());
} // dump


std::string HttpRequest::getMethod() {
	return m_parser.getMethod();
} // getMethod


std::string HttpRequest::getPath() {
	return m_parser.getURL();
} // getURL

std::string HttpRequest::getRequestBody() {
	return m_parser.getBody();
} // getRequestBody


std::string HttpRequest::getRequestHeader(std::string name) {
	return m_parser.getHeader(name);
} // getRequestHeader


std::map<std::string, std::string> HttpRequest::getRequestHeaders() {
	return m_parser.getHeaders();
} // getRequestHeaders


std::string HttpRequest::getResponseHeader(std::string name) {
	if (m_responseHeaders.find(name) == m_responseHeaders.end()) {
		return "";
	}
	return m_responseHeaders.at(name);
} // getResponseHeader


std::map<std::string, std::string> HttpRequest::getResponseHeaders() {
	return m_responseHeaders;
} // getResponseHeaders


Socket HttpRequest::getSocket() {
		return m_clientSocket;
}





std::string HttpRequest::getVersion() {
	return m_parser.getVersion();
} // getVersion


bool HttpRequest::isWebsocket() {
	return m_isWebsocket;
}


void HttpRequest::sendResponse() {
	std::ostringstream oss;
	oss << getVersion() << " " << m_status << " " << m_responseMessage << lineTerminator;
	for (auto it = m_responseHeaders.begin(); it != m_responseHeaders.end(); ++it) {
		oss << it->first.c_str() << ": " << it->second.c_str() << lineTerminator;
	}
	oss << lineTerminator;
	oss << m_responseBody;
	ESP_LOGD(LOG_TAG, ">> sendResponse: %s", oss.str().c_str());
	m_clientSocket.send_cpp(oss.str());
	/*TODO*/
} // sendResponse


void HttpRequest::setResponseBody(const std::string body) {
	m_responseBody = body;
} // setResponseBody


void HttpRequest::setStatus(const int status, const std::string message) {
	m_status = status;
	m_responseMessage = message;
} // setStatus
