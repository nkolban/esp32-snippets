/*
 * WebServer.h
 *
 *  Created on: May 19, 2017
 *      Author: kolban
 */

#ifndef CPP_UTILS_WEBSERVER_H_
#define CPP_UTILS_WEBSERVER_H_
#include <string>
#include <vector>
#include <regex>
#include <map>
#include "sdkconfig.h"
#ifdef CONFIG_MONGOOSE_PRESENT
#include <mongoose.h>



class WebServer;

/**
 * @brief %WebServer built on Mongoose.
 *
 * A web server.
 */
class WebServer {
public:
	/**
	 * @brief Request wrapper for an HTTP request.
	 */
	class HTTPRequest {
		public:
			HTTPRequest(struct http_message* message);
			std::string getMethod();
			std::string getPath();
			std::map<std::string, std::string> getQuery();
			std::string getBody();
			std::vector<std::string> pathSplit();
		private:
			struct http_message* m_message;
	}; // HTTPRequest

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
			bool m_dataSent;
	}; // HTTPResponse

	/**
	 * @brief Handler for a Multipart.
	 *
	 * This class is usually subclassed to provide your own implementation.  Typically
	 * a factory is implemented based on HTTPMultiPartFactory that creates instances.  This
	 * is then registered with the WebServer.  When done, when ever the WebServer receives multi
	 * part requests, this handler for Multipart is called.  The call sequence is usually:
	 * ~~~
	 * multipartStart
	 * begin
	 * data*
	 * end
	 * begin
	 * data*
	 * end
	 * ...
	 * multipartEnd
	 * ~~~
	 *
	 * There can be multiple begin, data, data, ..., end groups.
	 */
	class HTTPMultiPart {
	public:
		virtual ~HTTPMultiPart() {
		};
		virtual void begin(std::string varName, std::string fileName);
		virtual void end();
		virtual void data(std::string data);
		virtual void multipartEnd();
		virtual void multipartStart();
	}; // HTTPMultiPart

	/**
	 * @brief Factory for creating Multipart instances.
	 * This class is meant to be implemented to provide a constructor for a custom
	 * HTTPMultiPart instance.
	 * @code{.cpp}
	 * class MyMultiPart : public WebServer::HTTPMultiPart {
	 * public:
	 *   void begin(std::string varName,	std::string fileName) {
	 *     ESP_LOGD(tag, "MyMultiPart begin(): varName=%s, fileName=%s",
	 *     varName.c_str(), fileName.c_str());
	 *   }
	 *
	 *   void end() {
	 *     ESP_LOGD(tag, "MyMultiPart end()");
	 *   }
	 *
	 *   void data(std::string data) {
	 *     ESP_LOGD(tag, "MyMultiPart data(): length=%d", data.length());
	 *   }
	 *
	 *   void multipartEnd() {
	 *     ESP_LOGD(tag, "MyMultiPart multipartEnd()");
	 *   }
	 *
	 *   void multipartStart() {
	 *     ESP_LOGD(tag, "MyMultiPart multipartStart()");
	 *   }
	 * };
	 *
	 * class MyMultiPartFactory : public WebServer::HTTPMultiPartFactory {
	 *   WebServer::HTTPMultiPart *createNew() {
	 *     return new MyMultiPart();
	 *   }
	 * };
	 * @endcode
	 */
	class HTTPMultiPartFactory {
	public:
		/**
		 * @brief Create a new HTTPMultiPart instance.
		 * @return A new HTTPMultiPart instance.
		 */
		virtual HTTPMultiPart *newInstance() = 0;
	};

	/**
	 * @brief The handler for path matching.
	 *
	 */
	class PathHandler {
		public:
			PathHandler(std::string method, std::string pathPattern,  void (*webServerRequestHandler)(WebServer::HTTPRequest *pHttpRequest, WebServer::HTTPResponse *pHttpResponse));
			bool match(std::string method, std::string path);
			void invoke(HTTPRequest *request, HTTPResponse *response);
		private:
			std::string m_method;
			std::regex m_pattern;
			void (*m_requestHandler)(WebServer::HTTPRequest *pHttpRequest, WebServer::HTTPResponse *pHttpResponse);
	}; // PathHandler

	/**
	 * @brief A WebSocket handler for handling WebSockets.
	 */
	class WebSocketHandler {
	public:
		void onCreated();
		virtual void onMessage(std::string message);
		void onClosed();
		void sendData(std::string message);
		void sendData(uint8_t *data, uint32_t size);
		void close();
	private:
		struct mg_connection *m_mgConnection;
	};

	class WebSocketHandlerFactory {
	public:
		virtual WebSocketHandler *newInstance() = 0;
	};

	WebServer();
	virtual ~WebServer();
	void addPathHandler(std::string method, std::string pathExpr, void (*webServerRequestHandler)(WebServer::HTTPRequest *pHttpRequest, WebServer::HTTPResponse *pHttpResponse) );
	std::string getRootPath();
	void setMultiPartFactory(HTTPMultiPartFactory *pMultiPartFactory);
	void setRootPath(std::string path);
	void setWebSocketHandlerFactory(WebSocketHandlerFactory *pWebSocketHandlerFactory);
	void start(unsigned short port = 80);
	void processRequest(struct mg_connection *mgConnection, struct http_message *message);
	HTTPMultiPartFactory *m_pMultiPartFactory;
	WebSocketHandlerFactory *m_pWebSocketHandlerFactory;
private:
	std::string m_rootPath;
	std::vector<PathHandler> m_pathHandlers;
};

#endif // CONFIG_MONGOOSE_PRESENT
#endif /* CPP_UTILS_WEBSERVER_H_ */
