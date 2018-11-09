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
#include <vector>
#include <algorithm>
#include "HttpResponse.h"
#include "HttpRequest.h"
#include "GeneralUtils.h"

#include <esp_log.h>
#include <hwcrypto/sha.h>

#define STATE_NAME  0
#define STATE_VALUE 1

static const char* LOG_TAG="HttpRequest";

//static std::string lineTerminator = "\r\n";

const char HttpRequest::HTTP_HEADER_ACCEPT[]         = "Accept";
const char HttpRequest::HTTP_HEADER_ALLOW[]          = "Allow";
const char HttpRequest::HTTP_HEADER_CONNECTION[]     = "Connection";
const char HttpRequest::HTTP_HEADER_CONTENT_LENGTH[] = "Content-Length";
const char HttpRequest::HTTP_HEADER_CONTENT_TYPE[]   = "Content-Type";
const char HttpRequest::HTTP_HEADER_COOKIE[]         = "Cookie";
const char HttpRequest::HTTP_HEADER_HOST[]           = "Host";
const char HttpRequest::HTTP_HEADER_LAST_MODIFIED[]  = "Last-Modified";
const char HttpRequest::HTTP_HEADER_ORIGIN[]         = "Origin";
const char HttpRequest::HTTP_HEADER_SEC_WEBSOCKET_ACCEPT[]   = "Sec-WebSocket-Accept";
const char HttpRequest::HTTP_HEADER_SEC_WEBSOCKET_PROTOCOL[] = "Sec-WebSocket-Protocol";
const char HttpRequest::HTTP_HEADER_SEC_WEBSOCKET_KEY[]      = "Sec-WebSocket-Key";
const char HttpRequest::HTTP_HEADER_SEC_WEBSOCKET_VERSION[]  = "Sec-WebSocket-Version";
const char HttpRequest::HTTP_HEADER_UPGRADE[]        = "Upgrade";
const char HttpRequest::HTTP_HEADER_USER_AGENT[]     = "User-Agent";

const char HttpRequest::HTTP_METHOD_CONNECT[] = "CONNECT";
const char HttpRequest::HTTP_METHOD_DELETE[]  = "DELETE";
const char HttpRequest::HTTP_METHOD_GET[]     = "GET";
const char HttpRequest::HTTP_METHOD_HEAD[]    = "HEAD";
const char HttpRequest::HTTP_METHOD_OPTIONS[] = "OPTIONS";
const char HttpRequest::HTTP_METHOD_PATCH[]   = "PATCH";
const char HttpRequest::HTTP_METHOD_POST[]    = "POST";
const char HttpRequest::HTTP_METHOD_PUT[]     = "PUT";



/**
 * @brief Build a WebSockey response has.
 */
std::string buildWebsocketKeyResponseHash(std::string requestKey) {
	std::string newKey = requestKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	uint8_t shaData[20];
	esp_sha(SHA1, (uint8_t*) newKey.data(), newKey.length(), shaData);
	//GeneralUtils::hexDump(shaData, 20);
	std::string retStr;
	GeneralUtils::base64Encode(std::string((char*) shaData, sizeof(shaData)), &retStr);
	return retStr;
} // buildWebsocketKeyResponseHash


/**
 * @brief Create an HTTP Request instance.
 */
HttpRequest::HttpRequest(Socket clientSocket) {
	m_clientSocket = clientSocket;
	m_pWebSocket   = nullptr;
	m_isClosed     = false;

	m_parser.parse(clientSocket); // Parse the socket stream to build the HTTP data.

	// We have to take some special action on the Connection header.  We want to know if it contains "Upgrade"
	// however it has come to light that the Connection header can contain multiple parts.  For example, it has
	// been reported that it can contain "keep-alive,Upgrade".  Because of this we can't simply examine the string
	// to see if it equals "Upgrade".  Our solution is to get the value of Connection string, split it by "," as
	// a delimiter and then examine each of the parts to see if any of those are "Upgrade".
	std::vector<std::string> parts = GeneralUtils::split(getHeader(HTTP_HEADER_CONNECTION), ',');
	bool upgradeFound = false;
	if (std::find(parts.begin(), parts.end(), "Upgrade") != parts.end()) {
		upgradeFound = true;
	}

	// Is this a Web Socket?
	if (getMethod() == HTTP_METHOD_GET &&
			!getHeader(HTTP_HEADER_HOST).empty() &&
			getHeader(HTTP_HEADER_UPGRADE) == "websocket" &&
			//getHeader(HTTP_HEADER_CONNECTION) == "Upgrade" &&
			upgradeFound &&
			!getHeader(HTTP_HEADER_SEC_WEBSOCKET_KEY).empty() &&
			!getHeader(HTTP_HEADER_SEC_WEBSOCKET_VERSION).empty()) {
		ESP_LOGD(LOG_TAG, "Websocket detected!");
		// do something
		// Process the web socket request

		// Build and send the response HTTP message to switch to being a Web Socket
		HttpResponse response(this);

		response.setStatus(HttpResponse::HTTP_STATUS_SWITCHING_PROTOCOL, "Switching Protocols");
		response.addHeader(HTTP_HEADER_UPGRADE, "websocket");
		response.addHeader(HTTP_HEADER_CONNECTION, "Upgrade");
		response.addHeader(HTTP_HEADER_SEC_WEBSOCKET_ACCEPT,
			buildWebsocketKeyResponseHash(getHeader(HTTP_HEADER_SEC_WEBSOCKET_KEY)));
		response.sendData("");

		// Now that we have converted the request into a WebSocket, create the new WebSocket entry.
		m_pWebSocket = new WebSocket(clientSocket);
	} // if this is a web socket ...
} // HttpRequest


HttpRequest::~HttpRequest() {
} // ~HttpRequest


/**
 * @brief Close the HttpRequest
 */
void HttpRequest::close() {
	if (isWebsocket()) {
		ESP_LOGW(LOG_TAG, "Request to close an HTTP Request but we think it is a web socket!");
	}
	m_clientSocket.close();
	m_isClosed = true;
} // close_cpp


/**
 * @brief Dump the HttpRequest for debugging purposes.
 */
void HttpRequest::dump() {
	ESP_LOGD(LOG_TAG, "Method: %s, URL: \"%s\", Version: %s", getMethod().c_str(), getPath().c_str(), getVersion().c_str());
	auto headers = getHeaders();
	auto it2 = headers.begin();
	for (; it2 != headers.end(); ++it2) {
		ESP_LOGD(LOG_TAG, "name=\"%s\", value=\"%s\"", it2->first.c_str(), it2->second.c_str());
	}
	ESP_LOGD(LOG_TAG, "Body: \"%s\"", getBody().c_str());
} // dump


/**
 * @brief Get the body of the HttpRequest.
 */
std::string HttpRequest::getBody() {
	return m_parser.getBody();
} // getBody


/**
 * @brief Get the named header.
 * @param [in] name The name of the header field to retrieve.
 * @return The value of the header field.
 */
std::string HttpRequest::getHeader(std::string name) {
	return m_parser.getHeader(name);
} // getHeader


std::map<std::string, std::string> HttpRequest::getHeaders() {
	return m_parser.getHeaders();
} // getHeaders


std::string HttpRequest::getMethod() {
	return m_parser.getMethod();
} // getMethod


std::string HttpRequest::getPath() {
	return m_parser.getURL();
} // getPath


/**
 * @brief Get the query part of the request.
 * The query is a set of name = value pairs.  The return is a map keyed by the name items.
 *
 * @return The query part of the request.
 */
std::map<std::string, std::string> HttpRequest::getQuery() {
	// Walk through all the characters in the query string maintaining a simple state machine
	// that lets us know what we are parsing.
	std::map<std::string, std::string> queryMap;

	std::string possibleQueryString = getPath();
	int qindex = possibleQueryString.find_first_of("?");
	if (qindex < 0) {
		ESP_LOGD(LOG_TAG, "No query string present");
		return queryMap ;
	}
	std::string queryString = possibleQueryString.substr(qindex + 1, -1) ;
	ESP_LOGD(LOG_TAG, "query string: %s", queryString.c_str()) ;

	/*
	 * We maintain a simple state machine with states of:
	 * * STATE_NAME - We are parsing a name.
	 * * STATE_VALUE - We are parsing a value.
	 */
	int state = STATE_NAME;
	std::string name = "";
	std::string value;
	// Loop through each character in the query string.
	for (int i = 0; i < queryString.length(); i++) {
		char currentChar = queryString[i];
		if (state == STATE_NAME) {
			if (currentChar != '=') {
				name += currentChar;
			} else {
				state = STATE_VALUE;
				value = "";
			}
		} // End state = STATE_NAME
		else if (state == STATE_VALUE) {
			if (currentChar != '&') {
				value += currentChar;
			} else {
				//ESP_LOGD(tag, "name=%s, value=%s", name.c_str(), value.c_str());
				queryMap[name] = value;
				state = STATE_NAME;
				name = "";
			}
		} // End state = STATE_VALUE
	} // End for loop
	if (state == STATE_VALUE) {
		//ESP_LOGD(tag, "name=%s, value=%s", name.c_str(), value.c_str());
		queryMap[name] = value;
	}
	return queryMap;
} // getQuery


/**
 * @brief Get the underlying socket.
 * @return The underlying socket.
 */
Socket HttpRequest::getSocket() {
		return m_clientSocket;
} // getSocket


std::string HttpRequest::getVersion() {
	return m_parser.getVersion();
} // getVersion


WebSocket* HttpRequest::getWebSocket() {
	return m_pWebSocket;
} // getWebSocket


/**
 * @brief Determine if the request is closed.
 * @return Returns true if the request is closed.
 */
bool HttpRequest::isClosed() {
	return m_isClosed;
} // isClosed


/**
 * @brief Determine if this request represents a WebSocket
 * @return True if the request creates a web socket.
 */
bool HttpRequest::isWebsocket() {
	return m_pWebSocket != nullptr;
} // isWebsocket


/**
 * @brief Parse the request message as a form.
 * @return A map containing the names/values of the form elements that were found.
 */
std::map<std::string, std::string> HttpRequest::parseForm() {
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
std::vector<std::string> HttpRequest::pathSplit() {
	std::istringstream stream(getPath());
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
 * @brief Decode a URL/form
 * @param [in] str
 * @return The decoded string.
 */
std::string HttpRequest::urlDecode(std::string str) {
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
