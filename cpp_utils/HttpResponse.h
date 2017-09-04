/*
 * HttpResponse.h
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
private:
	HttpRequest* m_request;
	std::string  m_statusMessage;
	int          m_status;
	bool         m_headerCommitted;
	std::map<std::string, std::string> m_responseHeaders;
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

	void addHeader(std::string name, std::string value);
	void close_cpp();
	//std::string getRootPath();
	std::string getHeader(std::string name);
	std::map<std::string, std::string> getHeaders();
	void sendData(std::string data);
	//void sendData(uint8_t *pData, size_t length);
	//void setHeaders(std::map<std::string, std::string>  headers);
	void setStatus(int status, std::string message);
	//void setRootPath(std::string path);
};

#endif /* COMPONENTS_CPP_UTILS_HTTPRESPONSE_H_ */
