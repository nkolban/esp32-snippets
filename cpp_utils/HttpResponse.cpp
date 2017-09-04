/*
 * HttpResponse.cpp
 *
 *  Created on: Sep 2, 2017
 *      Author: kolban
 */
#include <sstream>
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <esp_log.h>

//static const char* LOG_TAG = "HttpResponse";

const int HttpResponse::HTTP_STATUS_CONTINUE              = 100;
const int HttpResponse::HTTP_STATUS_SWITCHING_PROTOCOL    = 101;
const int HttpResponse::HTTP_STATUS_OK                    = 200;
const int HttpResponse::HTTP_STATUS_MOVED_PERMANENTLY     = 301;
const int HttpResponse::HTTP_STATUS_BAD_REQUEST           = 400;
const int HttpResponse::HTTP_STATUS_UNAUTHORIZED          = 401;
const int HttpResponse::HTTP_STATUS_FORBIDDEN             = 403;
const int HttpResponse::HTTP_STATUS_NOT_FOUND             = 404;
const int HttpResponse::HTTP_STATUS_METHOD_NOT_ALLOWED    = 405;
const int HttpResponse::HTTP_STATUS_INTERNAL_SERVER_ERROR = 500;
const int HttpResponse::HTTP_STATUS_NOT_IMPLEMENTED       = 501;
const int HttpResponse::HTTP_STATUS_SERVICE_UNAVAILABLE   = 503;

static std::string lineTerminator = "\r\n";
HttpResponse::HttpResponse(HttpRequest *request) {
	m_request = request;
	m_status  = 0;
	m_headerCommitted = false; // We have not yet sent a header.
}

HttpResponse::~HttpResponse() {
	// TODO Auto-generated destructor stub
}


/**
 * @brief Add a header to the response message.
 * If the response has already been committed then ignore this request.
 * @param [in] name The name of the header.
 * @param [in] value The value of the header.
 */
void HttpResponse::addHeader(const std::string name, const std::string value) {
	if (m_headerCommitted) {
		return;
	}
	m_responseHeaders.insert(std::pair<std::string, std::string>(name, value));
} // addHeader


void HttpResponse::close_cpp() {
	m_request->close_cpp();
} // close_cpp


std::string HttpResponse::getHeader(std::string name) {
	if (m_responseHeaders.find(name) == m_responseHeaders.end()) {
		return "";
	}
	return m_responseHeaders.at(name);
} // getHeader


std::map<std::string, std::string> HttpResponse::getHeaders() {
	return m_responseHeaders;
} // getHeaders


/**
 * @brief Send data to the partner.
 * Send some data to the partner.  If we haven't yet sent the HTTP header then send that now.
 * @param [in] data The data to send to the partner.
 */
void HttpResponse::sendData(std::string data) {
	if (m_headerCommitted == false) {
		std::ostringstream oss;
		oss << m_request->getVersion() << " " << m_status << " " << m_statusMessage << lineTerminator;
		for (auto it = m_responseHeaders.begin(); it != m_responseHeaders.end(); ++it) {
			oss << it->first.c_str() << ": " << it->second.c_str() << lineTerminator;
		}
		oss << lineTerminator;
		m_headerCommitted = true;
		m_request->getSocket().send_cpp(oss.str());
	}
	m_request->getSocket().send_cpp(data);
} // sendData


void HttpResponse::setStatus(const int status, const std::string message) {
	if (m_headerCommitted) {
		return;
	}
	m_status = status;
	m_statusMessage = message;
} // setStatus

