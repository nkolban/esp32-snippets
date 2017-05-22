/*
 * WebServer.h
 *
 *  Created on: May 19, 2017
 *      Author: kolban
 */

#ifndef CPP_UTILS_WEBSERVER_H_
#define CPP_UTILS_WEBSERVER_H_
#include <string>
#include <map>
#include "sdkconfig.h"
#ifdef CONFIG_MONGOOSE_PRESENT
#include <mongoose.h>
/**
 * @brief %WebServer built on Mongoose.
 *
 * A web server.
 */
class WebServer {
public:
	WebServer();
	virtual ~WebServer();
	std::string getRootPath();
	void run(unsigned short port = 80);
	void setRootPath(std::string path);
private:
	std::string m_rootPath;
};

/**
 * @brief Response wrapper for an HTTP response.
 */
class HTTPResponse {
public:
	HTTPResponse(struct mg_connection *nc);
	void addHeader(std::string name, std::string value);
	std::string getRootPath();
	void setStatus(int status);
	void setHeaders(std::map<std::string, std::string>  headers);
	void sendData(std::string data);
	void sendData(uint8_t *pData, size_t length);
	void setRootPath(std::string path);
private:
	struct mg_connection *m_nc;
	std::string m_rootPath;
	int m_status;
	std::map<std::string, std::string> m_headers;
};

#endif // CONFIG_MONGOOSE_PRESENT
#endif /* CPP_UTILS_WEBSERVER_H_ */
