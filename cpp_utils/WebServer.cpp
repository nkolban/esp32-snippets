/*
 * WebServer.cpp
 *
 *  Created on: May 19, 2017
 *      Author: kolban
 */
#include <sstream>
#include <vector>
#include "sdkconfig.h"
#ifdef CONFIG_MONGOOSE_PRESENT
#include "WebServer.h"
#include <esp_log.h>
#include <mongoose.h>
#include <string>


static char tag[] = "WebServer";

/**
 * @brief Convert a Mongoose event type to a string.
 * @param [in] event The received event type.
 * @return The string representation of the event.
 */
static std::string mongoose_eventToString(int event) {
	switch (event) {
	case MG_EV_CONNECT:
		return "MG_EV_CONNECT";
	case MG_EV_ACCEPT:
		return "MG_EV_ACCEPT";
	case MG_EV_CLOSE:
		return "MG_EV_CLOSE";
	case MG_EV_SEND:
		return "MG_EV_SEND";
	case MG_EV_RECV:
		return "MG_EV_RECV";
	case MG_EV_POLL:
		return "MG_EV_POLL";
	case MG_EV_TIMER:
		return "MG_EV_TIMER";
	case MG_EV_HTTP_REQUEST:
		return "MG_EV_HTTP_REQUEST";
	case MG_EV_HTTP_REPLY:
		return "MG_EV_HTTP_REPLY";
	case MG_EV_HTTP_CHUNK:
		return "MG_EV_HTTP_CHUNK";
	case MG_EV_MQTT_CONNACK:
		return "MG_EV_MQTT_CONNACK";
	case MG_EV_MQTT_CONNECT:
		return "MG_EV_MQTT_CONNECT";
	case MG_EV_MQTT_DISCONNECT:
		return "MG_EV_MQTT_DISCONNECT";
	case MG_EV_MQTT_PINGREQ:
		return "MG_EV_MQTT_PINGREQ";
	case MG_EV_MQTT_PINGRESP:
		return "MG_EV_MQTT_PINGRESP";
	case MG_EV_MQTT_PUBACK:
		return "MG_EV_MQTT_PUBACK";
	case MG_EV_MQTT_PUBCOMP:
		return "MG_EV_MQTT_PUBCOMP";
	case MG_EV_MQTT_PUBLISH:
		return "MG_EV_MQTT_PUBLISH";
	case MG_EV_MQTT_PUBREC:
		return "MG_EV_MQTT_PUBREC";
	case MG_EV_MQTT_PUBREL:
		return "MG_EV_MQTT_PUBREL";
	case MG_EV_MQTT_SUBACK:
		return "MG_EV_MQTT_SUBACK";
	case MG_EV_MQTT_SUBSCRIBE:
		return "MG_EV_MQTT_SUBSCRIBE";
	case MG_EV_MQTT_UNSUBACK:
		return "MG_EV_MQTT_UNSUBACK";
	case MG_EV_MQTT_UNSUBSCRIBE:
		return "MG_EV_MQTT_UNSUBSCRIBE";
	case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST:
		return "MG_EV_WEBSOCKET_HANDSHAKE_REQUEST";
	case MG_EV_WEBSOCKET_HANDSHAKE_DONE:
		return "MG_EV_WEBSOCKET_HANDSHAKE_DONE";
	case MG_EV_WEBSOCKET_FRAME:
		return "MG_EV_WEBSOCKET_FRAME";
	case MG_EV_WEBSOCKET_CONTROL_FRAME:
		return "MG_EV_WEBSOCKET_CONTROL_FRAME";
	}
	std::ostringstream s;
	s << "Unknown event: " << event;
	return s.str();
} //eventToString

/**
 * @brief Convert a Mongoose string type to a string.
 * @param [in] mgStr The Mongoose string.
 * @return A std::string representation of the Mongoose string.
 */
static std::string mgStrToString(struct mg_str mgStr) {
	return std::string(mgStr.p, mgStr.len);
} // mgStrToStr

static void dumpHttpMessage(struct http_message *pHttpMessage) {
	ESP_LOGD(tag, "HTTP Message");
	ESP_LOGD(tag, "Message: %s", mgStrToString(pHttpMessage->message).c_str());
	ESP_LOGD(tag, "URI: %s", mgStrToString(pHttpMessage->uri).c_str());
}

// Mongoose event handler.
static void mongoose_event_handler_web_server(struct mg_connection *mgConnection, int event, void *eventData) {
	if (event == MG_EV_POLL) {
		return;
	}
	ESP_LOGD(tag, "Event: %s", mongoose_eventToString(event).c_str());
	switch (event) {
	case MG_EV_HTTP_REQUEST: {
			ESP_LOGD(tag, "userData->%x", (uint32_t)mgConnection->user_data);
			struct http_message *message = (struct http_message *) eventData;
			ESP_LOGD(tag, "Event received");
			dumpHttpMessage(message);

			HTTPResponse httpResponse = HTTPResponse(mgConnection);
			WebServer *pWebServer = (WebServer *)mgConnection->user_data;
			httpResponse.setRootPath(pWebServer->getRootPath());

			std::string filePath = httpResponse.getRootPath() + mgStrToString(message->uri);
			ESP_LOGD(tag, "Opening file: %s", filePath.c_str());
			FILE *file = fopen(filePath.c_str(), "r");
			if (file != nullptr) {
				fseek(file, 0L, SEEK_END);
				size_t length = ftell(file);
				fseek(file, 0L, SEEK_SET);
				uint8_t *pData = (uint8_t *)malloc(length);
				fread(pData, length, 1, file);
				fclose(file);
				httpResponse.sendData(pData, length);
				free(pData);
			} else {
				// Handle unable to open file
				httpResponse.setStatus(404); // Not found
				httpResponse.sendData("");
			}

			break;
		}
	} // End of switch
} // End of mongoose_event_handler


WebServer::WebServer() {
	m_rootPath = "";
}


WebServer::~WebServer() {
}


/**
 * @brief Get the current root path.
 * @return The current root path.
 */
std::string WebServer::getRootPath() {
	return m_rootPath;
} // getRootPath


/**
 * @brief Run the web server listening at the given port.
 *
 * @param [in] port The port number of which to listen.
 * @return N/A.
 */
void WebServer::run(uint16_t port) {
	ESP_LOGD(tag, "Mongoose task starting");
	struct mg_mgr mgr;
	ESP_LOGD(tag, "Mongoose: Starting setup");
	mg_mgr_init(&mgr, NULL);
	ESP_LOGD(tag, "Mongoose: Succesfully inited");
	struct mg_connection *mgConnection = mg_bind(&mgr, ":80", mongoose_event_handler_web_server);
	mgConnection->user_data = this;
	ESP_LOGD(tag, "Mongoose Successfully bound");
	if (mgConnection == NULL) {
		ESP_LOGE(tag, "No connection from the mg_bind()");
		vTaskDelete(NULL);
		return;
	}
	mg_set_protocol_http_websocket(mgConnection);

	while (1) {
		mg_mgr_poll(&mgr, 2000);
	}
} // run


/**
 * @brief Set the root path for URL file mapping.
 *
 * When a browser requests a file, it uses the address form:
 *
 * @code{.unparsed}
 * http://<host>:<port>/<path>
 * @endcode
 *
 * The path part can be considered the path to where the file should be retrieved on the
 * file system available to the web server.  Typically, we want a directory structure on the file
 * system to host the web served files and not expose the whole file system.  Using this method
 * we specify the root directory from which the files will be served.
 *
 * @param [in] path The root path on the file system.
 * @return N/A.
 */
void WebServer::setRootPath(std::string path) {
	m_rootPath = path;
} // setRootPath


HTTPResponse::HTTPResponse(struct mg_connection* nc) {
	m_nc = nc;
	m_status = 200;
} // HTTPResponse


/**
 * @brief Add a header to the response.
 * @param [in] name The name of the header.
 * @param [in] value The value of the header.
 */
void HTTPResponse::addHeader(std::string name, std::string value) {
	m_headers[name] = value;
} // addHeader


/**
 * @brief Send data to the HTTP caller.
 * Send the data to the HTTP caller.  No further data should be sent after this call.
 * @param [in] data The data to be sent to the HTTP caller.
 * @return N/A.
 */
void HTTPResponse::sendData(std::string data) {
	sendData((uint8_t *)data.data(), data.length());
} // sendData


/**
 * @brief Send data to the HTTP caller.
 * Send the data to the HTTP caller.  No further data should be sent after this call.
 * @param [in] pData The data to be sent to the HTTP caller.
 * @param [in] length The length of the data to be sent.
 * @return N/A.
 */
void HTTPResponse::sendData(uint8_t *pData, size_t length) {
	std::map<std::string, std::string>::iterator iter;
	std::string headers;

	for (iter = m_headers.begin(); iter != m_headers.end(); iter++) {
		if (headers.length() == 0) {
			headers = iter->first + ": " + iter->second;
		} else {
			headers = "; " + iter->first + "=" + iter->second;
		}
	}
	mg_send_head(m_nc, m_status, length, headers.c_str());
	mg_send(m_nc, pData, length);
	m_nc->flags |= MG_F_SEND_AND_CLOSE;
} // sendData


/**
 * @brief Set the headers to be sent in the HTTP response.
 * @param [in] header The complete set of headers to send to the caller.
 * @return N/A.
 */
void HTTPResponse::setHeaders(std::map<std::string, std::string>  headers) {
	m_headers = headers;
} // setHeaders


/**
 * @brief Get the current root path.
 * @return The current root path.
 */
std::string HTTPResponse::getRootPath() {
	return m_rootPath;
} // getRootPath


/**
 * @brief Set the root path for URL file mapping.
 * @param [in] path The root path on the file system.
 * @return N/A.
 */
void HTTPResponse::setRootPath(std::string path) {
	m_rootPath = path;
} // setRootPath


/**
 * @brief Set the status value in the HTTP response.
 *
 * The default if not set is 200.
 * @param [in] status The HTTP status code sent to the caller.
 * @return N/A.
 */
void HTTPResponse::setStatus(int status) {
	m_status = status;
} // setStatus


#endif // CONFIG_MONGOOSE_PRESENT
