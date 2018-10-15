/*
 * HttpResponse.cpp
 *
 *  Created on: Sep 2, 2017
 *      Author: kolban
 */
#include <sstream>
#include <fstream>
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <esp_log.h>

static const char* LOG_TAG = "HttpResponse";

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
HttpResponse::HttpResponse(HttpRequest* request) {
	m_request = request;
	m_status  = 200;
	m_headerCommitted = false; // We have not yet sent a header.
}


HttpResponse::~HttpResponse() {
}


/**
 * @brief Add a header to the response message.
 * If the response has already been committed then ignore this request.
 * @param [in] name The name of the header.
 * @param [in] value The value of the header.
 */
void HttpResponse::addHeader(const std::string name, const std::string value) {
	if (m_headerCommitted) return;
	m_responseHeaders.insert(std::pair<std::string, std::string>(name, value));
} // addHeader


/**
 * @brief Close the response.
 * We close the response.  If we haven't yet sent the header, we send that now and then close
 * the socket.
 */
void HttpResponse::close() {
	// If we haven't yet sent the header of the data, send that now.
	if (!m_headerCommitted) {
		sendHeader();
	}
	m_request->close();
} // close


/**
 * @brief Get the value of the named header.
 * @param [in] name The name of the header for which the value is to be returned.
 * @return The value of the named header.
 */
std::string HttpResponse::getHeader(std::string name) {
	if (m_responseHeaders.find(name) == m_responseHeaders.end()) return "";
	return m_responseHeaders.at(name);
} // getHeader


std::map<std::string, std::string> HttpResponse::getHeaders() {
	return m_responseHeaders;
} // getHeaders


/**
 * @brief Send data to the partner.
 * Send some data to the partner.  If we haven't yet sent the HTTP header then send that now.  We can call this function
 * any number of times before calling close.  After calling close, we may not call this function again.
 * @param [in] data The data to send to the partner.
 */
void HttpResponse::sendData(std::string data) {
	ESP_LOGD(LOG_TAG, ">> sendData");
	// If the request is already closed, nothing further to do.
	if (m_request->isClosed()) {
		ESP_LOGE(LOG_TAG, "<< sendData: Request to send more data but the request/response is already closed");
		return;
	}

	// If we haven't yet sent the header of the data, send that now.
	if (!m_headerCommitted) {
		sendHeader();
	}

	// Send the payload data.
	m_request->getSocket().send(data);
	ESP_LOGD(LOG_TAG, "<< sendData");
} // sendData

void HttpResponse::sendData(uint8_t* pData, size_t size) {
	ESP_LOGD(LOG_TAG, ">> sendData: 0x%x, size: %d", (uint32_t) pData, size);
	// If the request is already closed, nothing further to do.
	if (m_request->isClosed()) {
		ESP_LOGE(LOG_TAG, "<< sendData: Request to send more data but the request/response is already closed");
		return;
	}

	// If we haven't yet sent the header of the data, send that now.
	if (!m_headerCommitted) {
		sendHeader();
	}

	// Send the payload data.
	m_request->getSocket().send(pData, size);
	ESP_LOGD(LOG_TAG, "<< sendData");
} // sendData

void HttpResponse::sendFile(std::string fileName, size_t bufSize) {
	ESP_LOGI(LOG_TAG, "Opening file: %s", fileName.c_str());
	std::ifstream ifStream;
	ifStream.open(fileName, std::ifstream::in | std::ifstream::binary);      // Attempt to open the file for reading.

	// If we failed to open the requested file, then it probably didn't exist so return a not found.
	if (!ifStream.is_open()) {
		ESP_LOGE(LOG_TAG, "Unable to open file %s for reading", fileName.c_str());
		setStatus(HttpResponse::HTTP_STATUS_NOT_FOUND, "Not Found");
		addHeader(HttpRequest::HTTP_HEADER_CONTENT_TYPE, "text/plain");
		sendData("Not Found");
		close();
		return; // Since we failed to open the file, no further work to be done.
	}

	// We now have an open file and want to push the content of that file through to the browser.
	// because of defect #252 we have to do some pretty important re-work here.  Specifically, we can't host the whole file in
	// RAM at one time.  Instead what we have to do is ensure that we only have enough data in RAM to be sent.

	setStatus(HttpResponse::HTTP_STATUS_OK, "OK");
	uint8_t *pData = new uint8_t[bufSize];
	while (!ifStream.eof()) {
		ifStream.read((char*) pData, bufSize);
		sendData(pData, ifStream.gcount());
	}
	delete[] pData;
	ifStream.close();
	close();
} // sendFile

/**
 * @brief Send the header
 */
void HttpResponse::sendHeader() {
	// If we haven't yet sent the header of the data, send that now.
	if (!m_headerCommitted) {
		std::ostringstream oss;
		oss << m_request->getVersion() << " " << m_status << " " << m_statusMessage << lineTerminator;
		for (auto it = m_responseHeaders.begin(); it != m_responseHeaders.end(); ++it) {
			oss << it->first.c_str() << ": " << it->second.c_str() << lineTerminator;
		}
		oss << lineTerminator;
		m_headerCommitted = true;
		m_request->getSocket().send(oss.str());
	}
} // sendHeader


/**
 * @brief Set the status code that is to be sent back to the client.
 * When a client makes a request, the response contains a status.  This call sets the status that
 * will be sent back to the client.
 * @param [in] status The status code to be sent back to the caller.
 * @param [in] message The message to be sent with the status code.
 */
void HttpResponse::setStatus(const int status, const std::string message) {
	if (m_headerCommitted) { // If the header has already been sent, don't set the new values.
		ESP_LOGW(LOG_TAG, "Attempt to set a new status(%d)/message(%s) but header has already been committed.", status, message.c_str());
		return;
	}
	m_status        = status;
	m_statusMessage = message;
} // setStatus
