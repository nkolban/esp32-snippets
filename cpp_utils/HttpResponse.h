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
	HttpRequest m_httpRequest;
	std::string m_responseMessage;
	std::string m_responseBody;
	std::map<std::string, std::string> m_responseHeaders;
public:
	HttpResponse(HttpRequest httpRequest);
	virtual ~HttpResponse();


	void addHeader(std::string name, std::string value);
	//std::string getRootPath();

	void setHeaders(std::map<std::string, std::string>  headers);
	void setStatus(int status);
	void sendData(std::string data);
	void sendData(uint8_t *pData, size_t length);
	//void setRootPath(std::string path);
};

#endif /* COMPONENTS_CPP_UTILS_HTTPRESPONSE_H_ */
