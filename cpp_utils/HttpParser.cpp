/*
 * HttpParser.cpp
 *
 *  Created on: Aug 28, 2017
 *      Author: kolban
 */

#include <string>
#include <iostream>
#include <cstdlib>
#include "HttpParser.h"
#include "HttpRequest.h"
#include "GeneralUtils.h"

#include <esp_log.h>

#undef close
/**
 * RFC7230 - Hypertext Transfer Protocol (HTTP/1.1): Message Syntax and Routing
 * RFC3986 - URI
 *
 * <Request Line>
 * <Header>
 * <Header>
 * ...
 * <Header>
 * <CRLF>
 * <BODY>
 *
 * Example:
 * GET /hello.txt HTTP/1.1
 */

static const char* LOG_TAG = "HttpParser";

static std::string lineTerminator = "\r\n";


/**
 * @brief Parse an incoming line of text until we reach a delimiter.
 * @param [in/out] it The current iterator in the text input.
 * @param [in] str The string we are parsing.
 * @param [in] token The token delimiter.
 */
static std::string toStringToken(std::string::iterator& it, std::string& str, std::string& token) {
	std::string ret;
	std::string part;
	auto itToken = token.begin();
	for (; it != str.end(); ++it) {
		if ((*it) == (*itToken)) {
			part += (*itToken);
			++itToken;
			if (itToken == token.end()) {
				++it;
				break;
			}
		} else {
			if (part.empty()) {
				ret += part;
				part.clear();
				itToken = token.begin();
			}
			ret += *it;
		}
	} // for
	return ret;
} // toStringToken


/**
 * @brief Parse a string until the given token is found.
 * @param [in] it The current iterator.
 * @param [in] str The string being parsed.
 * @param [in] token The token terminating the parse.
 * @return The parsed string token.
 */
static std::string toCharToken(std::string::iterator& it, std::string& str, char token) {
	std::string ret;
	for(; it != str.end(); ++it) {
		if ((*it) == token) {
			++it;
			break;
		}
		ret += *it;
	}
	return ret;
} // toCharToken


/**
 * @brief Parse a header line.
 * An HTTP Header is of the form:
 *
 * Name":" Value
 *
 * We parse this and return a pair.
 * @param [in] line The line of text to parse.
 * @return A pair of the form name/value.
 */
std::pair<std::string, std::string> parseHeader(std::string& line) {
	auto it = line.begin();
	std::string name = toCharToken(it, line, ':'); // Parse the line until we find a ':'
	// We normalize the header name to be lower case.
	GeneralUtils::toLower(name);
	auto value = GeneralUtils::trim(toStringToken(it, line, lineTerminator));
	return std::pair<std::string, std::string>(name, value);
} // parseHeader


HttpParser::HttpParser() {
}

HttpParser::~HttpParser() {
}


/**
 * @brief Dump the outcome of the parse.
 *
 */
void HttpParser::dump() {
	ESP_LOGD(LOG_TAG, "Method: %s, URL: \"%s\", Version: %s", m_method.c_str(), m_url.c_str(), m_version.c_str());
	auto it2 = m_headers.begin();
	for (; it2 != m_headers.end(); ++it2) {
		ESP_LOGD(LOG_TAG, "name=\"%s\", value=\"%s\"", it2->first.c_str(), it2->second.c_str());
	}
	ESP_LOGD(LOG_TAG, "Body: \"%s\"", m_body.c_str());
} // dump


std::string HttpParser::getBody() {
	return m_body;
}


/**
 * @brief Retrieve the value of the named header.
 * @param [in] name The name of the header to retrieve.
 * @return The value of the named header or null if not present.
 */
std::string HttpParser::getHeader(const std::string& name) {
	// We normalize the header name to be lower case.
	std::string localName = name;
	GeneralUtils::toLower(localName);
	if (!hasHeader(localName)) return "";
	return m_headers.at(localName);
} // getHeader


std::map<std::string, std::string> HttpParser::getHeaders() {
	return m_headers;
} // getHeaders


std::string HttpParser::getMethod() {
	return m_method;
} // getMethod


std::string HttpParser::getURL() {
	return m_url;
} // getURL


std::string HttpParser::getVersion() {
	return m_version;
} // getVersion

std::string HttpParser::getStatus() {
	return m_status;
} // getStatus

std::string HttpParser::getReason() {
	return m_reason;
} // getReason

/**
 * @brief Determine if we have a header of the given name.
 * @param [in] name The name of the header to find.
 * @return True if the header is present and false otherwise.
 */
bool HttpParser::hasHeader(const std::string& name) {
	// We normalize the header name to be lower case.
	std::string localName = name;
	return m_headers.find(GeneralUtils::toLower(localName)) != m_headers.end();
} // hasHeader


/**
 * @brief Parse socket data.
 * @param [in] s The socket from which to retrieve data.
 */
void HttpParser::parse(Socket s) {
	ESP_LOGD(LOG_TAG, ">> parse: socket: %s", s.toString().c_str());
	std::string line;
	line = s.readToDelim(lineTerminator);
	parseRequestLine(line);
	line = s.readToDelim(lineTerminator);
	while (!line.empty()) {
		m_headers.insert(parseHeader(line));
		line = s.readToDelim(lineTerminator);
	}
	// Only PUT and POST requests have a body
	if (getMethod() != "POST" && getMethod() != "PUT") {
		ESP_LOGD(LOG_TAG, "<< parse");
		return;
	}

	// We have now parsed up to and including the separator ... we are now at the point where we
	// want to read the body.  There are two stories here.  The first is that we know the exact length
	// of the body or we read until we can't read anymore.
	if (hasHeader(HttpRequest::HTTP_HEADER_CONTENT_LENGTH)) {
		std::string val = getHeader(HttpRequest::HTTP_HEADER_CONTENT_LENGTH);
		int length = std::atoi(val.c_str());
		uint8_t data[length];
		s.receive(data, length, true);
		m_body = std::string((char*) data, length);
	} else {
		uint8_t data[512];
		int rc = s.receive(data, sizeof(data));
		if (rc > 0) {
			m_body = std::string((char*) data, rc);
		}
	}
	ESP_LOGD(LOG_TAG, "<< parse: Size of body: %d", m_body.length());
} // parse


/**
 * @brief Parse a string message.
 * @param [in] message The HTTP message to parse.
 */
/*
void HttpParser::parse(std::string message) {
	auto it = message.begin();
	auto line = toStringToken(it, message, lineTerminator);
	parseRequestLine(line);

	line = toStringToken(it, message, lineTerminator);
	while(!line.empty()) {
		//ESP_LOGD(LOG_TAG, "Header: \"%s\"", line.c_str());
		m_headers.insert(parseHeader(line));
		line = toStringToken(it, message, lineTerminator);
	}

	m_body = message.substr(std::distance(message.begin(), it));
} // parse
*/

/**
 * @brief Parse A request line.
 * @param [in] line The request line to parse.
 */
void HttpParser::parseRequestLine(std::string& line) {
	// A request Line is built from:
	// <method> <sp> <request-target> <sp> <HTTP-version>
	ESP_LOGD(LOG_TAG, ">> parseRequestLine: \"%s\" [%d]", line.c_str(), line.length());
	std::string::iterator it = line.begin();

	// Get the method
	m_method = toCharToken(it, line, ' ');

	// Get the url
	m_url = toCharToken(it, line, ' ');

	// Get the version
	m_version = toCharToken(it, line, ' ');
	ESP_LOGD(LOG_TAG, "<< parseRequestLine: method: %s, url: %s, version: %s", m_method.c_str(), m_url.c_str(), m_version.c_str());
} // parseRequestLine

/**
 * @brief Parse a response message.
 * @param [in] line The response to parse.
 */
void HttpParser::parseResponse(std::string message) {
	// A response is built from:
	// A status line, any number of header lines, a body
	auto it = message.begin();
	auto line = toStringToken(it, message, lineTerminator);
	parseStatusLine(line);

	line = toStringToken(it, message, lineTerminator);
	while (!line.empty()) {
		ESP_LOGD(LOG_TAG, "Header: \"%s\"", line.c_str());
		m_headers.insert(parseHeader(line));
		line = toStringToken(it, message, lineTerminator);
	}

	m_body = message.substr(std::distance(message.begin(), it));
} // parse

/**
 * @brief Parse A status line.
 * @param [in] line The status line to parse.
 */
void HttpParser::parseStatusLine(std::string& line) {
	// A status Line is built from:
	// <HTTP-version> <sp> <status> <sp> <reason>
	ESP_LOGD(LOG_TAG, ">> ParseStatusLine: \"%s\" [%d]", line.c_str(), line.length());
	std::string::iterator it = line.begin();
	// Get the version
	m_version = toCharToken(it, line, ' ');
	// Get the version
	m_status = toCharToken(it, line, ' ');
	// Get the status code
	m_reason = toStringToken(it, line, lineTerminator);

	ESP_LOGD(LOG_TAG, "<< ParseStatusLine: method: %s, version: %s, status: %s", m_method.c_str(), m_version.c_str(), m_status.c_str());
} // parseRequestLine
