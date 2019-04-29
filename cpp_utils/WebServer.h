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
#include "GeneralUtils.h"
#include "sdkconfig.h"

#ifdef CONFIG_MONGOOSE_PRESENT
#include "mongoose.h"

#define MAX_CHUNK_LENGTH 4090 // 4 kilobytes

typedef std::map<std::string, std::string> HeaderMap;
typedef std::pair<std::string, std::string> HeaderPair;

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
			static const char HTTP_HEADER_ACCEPT[];
			static const char HTTP_HEADER_ALLOW[];
			static const char HTTP_HEADER_CONNECTION[];
			static const char HTTP_HEADER_CONTENT_LENGTH[];
			static const char HTTP_HEADER_CONTENT_TYPE[];
			static const char HTTP_HEADER_CONTENT_ENCODING[];
			static const char HTTP_HEADER_COOKIE[];
			static const char HTTP_HEADER_HOST[];
			static const char HTTP_HEADER_LAST_MODIFIED[];
			static const char HTTP_HEADER_ORIGIN[];
			static const char HTTP_HEADER_SEC_WEBSOCKET_ACCEPT[];
			static const char HTTP_HEADER_SEC_WEBSOCKET_PROTOCOL[];
			static const char HTTP_HEADER_SEC_WEBSOCKET_KEY[];
			static const char HTTP_HEADER_SEC_WEBSOCKET_VERSION[];
			static const char HTTP_HEADER_UPGRADE[];
			static const char HTTP_HEADER_USER_AGENT[];

			static const char HTTP_METHOD_CONNECT[];
			static const char HTTP_METHOD_DELETE[];
			static const char HTTP_METHOD_GET[];
			static const char HTTP_METHOD_HEAD[];
			static const char HTTP_METHOD_OPTIONS[];
			static const char HTTP_METHOD_PATCH[];
			static const char HTTP_METHOD_POST[];
			static const char HTTP_METHOD_PUT[];

			const char* getMethod() const;
			const char* getPath() const;
			const char* getBody() const;
			size_t getMethodLen() const;
			size_t getPathLen() const;
			size_t getBodyLen() const;
			std::map<std::string, std::string> getQuery() const;
			std::vector<std::string> pathSplit() const;
			HeaderMap getHeaders() const;
			std::string getHeader(std::string) const;
			std::map<std::string, std::string> parseForm();
			std::string urlDecode(std::string str);

		private:
			struct http_message* m_message;
			HeaderMap m_headers;

	}; // HTTPRequest

	/**
	 * @brief Response wrapper for an HTTP response.
	 */
	class HTTPResponse {
		public:
			HTTPResponse(struct mg_connection* nc);

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

			void addHeader(const std::string& name, const std::string& value);
			void addHeader(std::string&& name, std::string&& value);
			void setStatus(int status);
			void setHeaders(const std::map<std::string, std::string>& headers);
			void setHeaders(std::map<std::string, std::string>&& headers);
			void sendData(const std::string& data);
			void sendData(const uint8_t* pData, size_t length);
			void sendData(const char* pData, size_t length);
			const std::string& getRootPath() const;
			void setRootPath(const std::string& path);
			void setRootPath(std::string&& path);
			void sendChunkHead();
			void sendChunk(const char* pData, size_t length);
			void closeConnection();

		private:
			struct mg_connection* m_nc;
			std::string m_rootPath;
			int m_status;
			std::map<std::string, std::string> m_headers;
			bool m_dataSent;
			std::string buildHeaders();

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
		virtual ~HTTPMultiPart() = default;
		virtual void begin(const std::string& varName, const std::string& fileName);
		virtual void end();
		virtual void data(const std::string& data);
		virtual void multipartEnd();
		virtual void multipartStart();
		virtual void setUri(std::string);
		virtual std::string getUri(void);
		virtual void setMethod(std::string);
		virtual std::string getMethod(void);
		virtual int getStatus(void);
	}; // HTTPMultiPart

	/**
	 * @brief Factory for creating Multipart instances.
	 * This class is meant to be implemented to provide a constructor for a custom
	 * HTTPMultiPart instance.
	 * @code{.cpp}
	 * class MyMultiPart : public WebServer::HTTPMultiPart {
	 * public:
	 *   void begin(std::string varName,	std::string fileName) {
	 *	 ESP_LOGD(tag, "MyMultiPart begin(): varName=%s, fileName=%s",
	 *	 varName.c_str(), fileName.c_str());
	 *   }
	 *
	 *   void end() {
	 *	 ESP_LOGD(tag, "MyMultiPart end()");
	 *   }
	 *
	 *   void data(std::string data) {
	 *	 ESP_LOGD(tag, "MyMultiPart data(): length=%d", data.length());
	 *   }
	 *
	 *   void multipartEnd() {
	 *	 ESP_LOGD(tag, "MyMultiPart multipartEnd()");
	 *   }
	 *
	 *   void multipartStart() {
	 *	 ESP_LOGD(tag, "MyMultiPart multipartStart()");
	 *   }
	 * };
	 *
	 * class MyMultiPartFactory : public WebServer::HTTPMultiPartFactory {
	 *   WebServer::HTTPMultiPart *createNew() {
	 *	 return new MyMultiPart();
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
		virtual HTTPMultiPart* newInstance() = 0;

	};

	/**
	 * @brief The handler for path matching.
	 *
	 */
	class PathHandler {
		public:
			PathHandler(const std::string& method, const std::string& pathPattern, void (*webServerRequestHandler) (WebServer::HTTPRequest* pHttpRequest, WebServer::HTTPResponse* pHttpResponse));
			PathHandler(std::string&& method, const std::string& pathPattern, void (*webServerRequestHandler) (WebServer::HTTPRequest* pHttpRequest, WebServer::HTTPResponse* pHttpResponse));
			bool match(const char* method, size_t method_len, const char* path);
			void invoke(HTTPRequest* request, HTTPResponse* response);

		private:
			std::string m_method;
			std::regex m_pattern;
			void (*m_requestHandler)(WebServer::HTTPRequest* pHttpRequest, WebServer::HTTPResponse* pHttpResponse);

	}; // PathHandler

	/**
	 * @brief A WebSocket handler for handling WebSockets.
	 */
	class WebSocketHandler {
	public:
		void onCreated();
		virtual void onMessage(const std::string& message);
		void onClosed();
		void sendData(const std::string& message);
		void sendData(const uint8_t* data, uint32_t size);
		void close();

	private:
		struct mg_connection* m_mgConnection;

	};

	class WebSocketHandlerFactory {
	public:
		virtual WebSocketHandler* newInstance() = 0;

	};

	WebServer();
	virtual ~WebServer();
	void addPathHandler(const std::string& method, const std::string& pathExpr, void (*webServerRequestHandler) (WebServer::HTTPRequest* pHttpRequest, WebServer::HTTPResponse* pHttpResponse));
	void addPathHandler(std::string&& method, const std::string& pathExpr, void (*webServerRequestHandler) (WebServer::HTTPRequest* pHttpRequest, WebServer::HTTPResponse* pHttpResponse));
	const std::string& getRootPath();
	void setMultiPartFactory(HTTPMultiPartFactory* pMultiPartFactory);
	void setRootPath(const std::string& path);
	void setRootPath(std::string&& path);
	void setWebSocketHandlerFactory(WebSocketHandlerFactory* pWebSocketHandlerFactory);
	void start(unsigned short port = 80);
	void processRequest(struct mg_connection* mgConnection, struct http_message* message);
	void processMultiRequest(struct mg_connection* mgConnection, struct http_message* message);
	void continueConnection(struct mg_connection* mgConnection);
	HTTPMultiPartFactory* m_pMultiPartFactory;
	WebSocketHandlerFactory* m_pWebSocketHandlerFactory;

private:
	std::string m_rootPath;
	std::vector<PathHandler> m_pathHandlers;
	std::map<sock_t, FILE*> unfinishedConnection;

};

#endif // CONFIG_MONGOOSE_PRESENT
#endif /* CPP_UTILS_WEBSERVER_H_ */
