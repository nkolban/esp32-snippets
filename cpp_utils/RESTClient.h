/*
 * RESTClient.h
 *
 *  Created on: Mar 12, 2017
 *      Author: kolban
 */

#ifndef MAIN_RESTCLIENT_H_
#define MAIN_RESTCLIENT_H_
#include "sdkconfig.h"
#if defined(CONFIG_LIBCURL_PRESENT)

#include <string>
#include <curl/curl.h>
class RESTClient;

/**
 * @brief Timing data for REST calls.
 */
class RESTTimings {
public:
	RESTTimings(RESTClient* client);
	void refresh();
	std::string toString();

private:
	double m_namelookup = 0;
	double m_connect = 0;
	double m_appconnect = 0;
	double m_pretransfer = 0;
	double m_starttransfer = 0;
	double m_total = 0;
	RESTClient* client = nullptr;

};

/**
 * @brief Encapsulate a REST client call.
 *
 * REST is the ability to make service calls using TCP and the HTTP protocols.  This class provides a
 * REST client encapsulation.  A REST client is the part of the story that makes calls to a partner
 * REST service provider (or server).  To make a call to a REST provider we need to specify:
 *
 * * The endpoint ... i.e. where are we sending the request.
 * * The protocol ... i.e. plain HTTP or secure HTTPS.
 * * The HTTP command ... i.e. one of GET, PUT, POST etc.
 * * The headers to the HTTP request.
 * * An optional payload for command types that accept payloads.
 *
 * Here is a code example:
 *
 * @code{cpp}
 * #include <RESTClient.h>
 *
 * RESTClient client;
 * client.setURL("https://httpbin.org/post");
 * client.addHeader("Content-Type", "application/json");
 * client.post("{ \"greeting\": \"hello world!\"");
 * @endcode
 *
 * To use this class you **must** define the `ESP_HAVE_CURL` build definition.  In your component.mk file
 * add:
 *
 * ```
 * CXXFLAGS+=-DESP_HAVE_CURL
 * ```
 */
class RESTClient {
public:
	RESTClient();
	virtual ~RESTClient();
	void addHeader(std::string name, std::string value);
	long get(); // Added return response_code 2018_4_12
	std::string getErrorMessage();
	/**
	 * @brief Get the response payload data from the last REST call.
	 *
	 * @return The response payload data.
	 */
	std::string getResponse() {
		return m_response;
	}

	/**
	 * @brief Get the timing information associated with this REST connection.
	 */
	RESTTimings *getTimings() {
		return m_timings;
	}

	long post(std::string body); // Added return response_code 2018_4_12

	/**
	 * @brief Set the URL for the target.
	 *
	 * @param [in] url The target for HTTP request.
	 *
	 */
	void setURL(std::string url) {
		m_url = url;
	};

	/**
	 * @brief Set the verbosity flag.
	 *
	 * @param [in] The value of the verbosity.
	 */
	void setVerbose(bool value) {
		m_verbose = value;
	};

private:
	CURL* m_curlHandle;
	std::string m_url;
	char m_errbuf[CURL_ERROR_SIZE];
	struct curl_slist* m_headers = nullptr;
	bool m_verbose = false;
	friend class RESTTimings;
	RESTTimings* m_timings;
	std::string m_response;
	static size_t handleData(void* buffer, size_t size, size_t nmemb, void* userp);
	void prepForCall();

};
#endif /* CONFIG_LIBCURL_PRESENT */
#endif /* MAIN_RESTCLIENT_H_ */
