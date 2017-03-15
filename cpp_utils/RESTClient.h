/*
 * RESTClient.h
 *
 *  Created on: Mar 12, 2017
 *      Author: kolban
 */

#ifndef MAIN_RESTCLIENT_H_
#define MAIN_RESTCLIENT_H_
#include <string>
#include <curl/curl.h>
class RESTClient;

class RESTTimings {
public:
	RESTTimings(RESTClient *client);
	void refresh();
	std::string toString();
private:
	double m_namelookup = 0;
	double m_connect = 0;
	double m_appconnect = 0;
	double m_pretransfer = 0;
	double m_starttransfer = 0;
	double m_total = 0;
	RESTClient *client = nullptr;
};

class RESTClient {
public:
	RESTClient();
	virtual ~RESTClient();
	void addHeader(std::string name, std::string value);
	void get();
	std::string getErrorMessage();
	std::string getResponse() {
		return m_response;
	}

	RESTTimings *getTimings() {
		return m_timings;
	}

	void post(std::string body);

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
	CURL *m_curlHandle;
	std::string m_url;
	char m_errbuf[CURL_ERROR_SIZE];
	struct curl_slist *m_headers = nullptr;
	bool m_verbose = false;
	friend class RESTTimings;
	RESTTimings *m_timings;
	std::string m_response;
	static size_t handleData(void *buffer, size_t size, size_t nmemb, void *userp);
	void prepForCall();
};

#endif /* MAIN_RESTCLIENT_H_ */
