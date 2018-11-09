/*
 * HttpResponse.h
 *
 * Encapsulate a response to be sent to the Http client.
 *
 *  Created on: Sep 2, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_HTTPRESPONSE_H_
#define COMPONENTS_CPP_UTILS_HTTPRESPONSE_H_
#include <string>
#include <map>
#include "HttpRequest.h"

class HttpResponse {
public:
	static const int HTTP_STATUS_CONTINUE;
	static const int HTTP_STATUS_SWITCHING_PROTOCOL;
	static const int HTTP_STATUS_OK;
	static const int HTTP_STATUS_MOVED_PERMANENTLY;
	static const int HTTP_STATUS_BAD_REQUEST;
	static const int HTTP_STATUS_UNAUTHORIZED;
	static const int HTTP_STATUS_FORBIDDEN;
	static const int HTTP_STATUS_NOT_FOUND;
	static const int HTTP_STATUS_METHOD_NOT_ALLOWED;
	static const int HTTP_STATUS_INTERNAL_SERVER_ERROR;
	static const int HTTP_STATUS_NOT_IMPLEMENTED;
	static const int HTTP_STATUS_SERVICE_UNAVAILABLE;

	HttpResponse(HttpRequest* httpRequest);
	virtual ~HttpResponse();

	void                               addHeader(std::string name, std::string value);  // Add a header to be sent to the client.
	void                               close();                                         // Close the request/response.
	std::string                        getHeader(std::string name);                     // Get a named header.
	std::map<std::string, std::string> getHeaders();                                    // Get all headers.
	void                               sendData(std::string data);                      // Send data to the client.
	void                               sendData(uint8_t* pData, size_t size);           // Send data to the client.
	void                               setStatus(int status, std::string message);      // Set the response status.
	void 							   sendFile(std::string fileName, size_t bufSize = 4 * 1024);	// Send file contents if exists.

private:
	bool							   m_headerCommitted;  // Has the header been sent?
	HttpRequest*					   m_request;		  // The request associated with this response.
	std::map<std::string, std::string> m_responseHeaders;  // The headers to be sent with the response.
	int								m_status;		   // The status to be sent with the response.
	std::string						m_statusMessage;	// The status message to be sent with the response.

	void sendHeader();									 // Send the header to the client.

};

#endif /* COMPONENTS_CPP_UTILS_HTTPRESPONSE_H_ */
