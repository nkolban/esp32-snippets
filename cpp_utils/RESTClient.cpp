/*
 * RESTClient.cpp
 *
 *  Created on: Mar 12, 2017
 *      Author: kolban
 */

#include "sdkconfig.h"
#if defined(CONFIG_LIBCURL_PRESENT)

#define _GLIBCXX_USE_C99 // Needed for std::string -> to_string inclusion.


#include <curl/curl.h>
#include <esp_log.h>
#include <string>

#include "RESTClient.h"

static const char* LOG_TAG = "RESTClient";


RESTClient::RESTClient() {
	m_curlHandle = curl_easy_init();
	m_timings = new RESTTimings(this);
} // RESTClient


RESTClient::~RESTClient() {
	::curl_easy_cleanup(m_curlHandle);
	curl_slist_free_all(m_headers);
	delete m_timings;
} // ~RESTClient


/**
 * @brief Perform an HTTP GET request.
 */
long RESTClient::get() {
	long response_code;  // Added return response_code 2018_4_12
	prepForCall();
	::curl_easy_setopt(m_curlHandle, CURLOPT_HTTPGET, 1);
	int rc = ::curl_easy_perform(m_curlHandle);
	if (rc != CURLE_OK) {
		ESP_LOGE(LOG_TAG, "get(): %s", getErrorMessage().c_str());
	}
	curl_easy_getinfo(m_curlHandle, CURLINFO_RESPONSE_CODE, &response_code); // Added return response_code 2018_4_12
	return response_code; // Added return response_code 2018_4_12
} // get


/**
 * @brief Perform an HTTP POST request.
 *
 * @param [in] body The body of the payload to send with the post request.
 *
 */
long RESTClient::post(std::string body) {
	long response_code; // Added return response_code 2018_4_12
	prepForCall();
	::curl_easy_setopt(m_curlHandle, CURLOPT_POSTFIELDS, body.c_str());
	int rc = ::curl_easy_perform(m_curlHandle);
	if (rc != CURLE_OK) {
		ESP_LOGE(LOG_TAG, "post(): %s", getErrorMessage().c_str());
	}
	curl_easy_getinfo(m_curlHandle, CURLINFO_RESPONSE_CODE, &response_code);// Added return response_code 2018_4_12
	return response_code;// Added return response_code 2018_4_12
} // post


/**
 * @brief Get the last error message.
 */
std::string RESTClient::getErrorMessage() {
	std::string errMsg(m_errbuf);
	return errMsg;
} // getErrorMessage


/**
 * @brief Callback function to handle the data received.
 *
 * This is a callback function architected by libcurl to be called when data is received.
 * We append the data to an accumulating buffer.
 *
 * @param [in] buffer A buffer of records.
 * @param [in] size The size of a record.
 * @param [in] nmemb The number of records of unit `size`.
 * @param [in] userp A pointer to the RESTClient class instance.
 *
 * @return The number of bytes of data processed.
 */
size_t RESTClient::handleData(void* buffer, size_t size, size_t nmemb, void* userp) {
	//printf("handleData: size: %d, num: %d\n", size, nmemb);
	RESTClient* pClient = (RESTClient*) userp;
	pClient->m_response.append((const char*) buffer, size * nmemb);
	return size * nmemb;
} // handleData


/**
 * @brief Add a header to the list of headers.
 *
 * For example:
 *
 * @code{.cpp}
 * client.addHeader("Content-Type", "application/json");
 * @endcode
 *
 * @param [in] name The name of the header to be added.
 * @param [in] value The value of the header to be added.
 */
void RESTClient::addHeader(std::string name, std::string value) {
	std::string headerString = name + ": " + value;
	m_headers = curl_slist_append(m_headers, headerString.c_str());
} // addHeader


/**
 * @brief Prepare for a call using a reset handle.
 */
void RESTClient::prepForCall() {
	::curl_easy_reset(m_curlHandle);

	if (m_verbose) {
		::curl_easy_setopt(m_curlHandle, CURLOPT_VERBOSE, 1L);
	} else {
		::curl_easy_setopt(m_curlHandle, CURLOPT_VERBOSE, 0L);
	}

	::curl_easy_setopt(m_curlHandle, CURLOPT_ERRORBUFFER, m_errbuf);
	::curl_easy_setopt(m_curlHandle, CURLOPT_SSL_VERIFYPEER, 0L);
	::curl_easy_setopt(m_curlHandle, CURLOPT_CAINFO, nullptr);
	::curl_easy_setopt(m_curlHandle, CURLOPT_URL, m_url.c_str());
	::curl_easy_setopt(m_curlHandle, CURLOPT_HTTPHEADER, m_headers);
	::curl_easy_setopt(m_curlHandle, CURLOPT_WRITEFUNCTION, handleData);
	::curl_easy_setopt(m_curlHandle, CURLOPT_WRITEDATA, this);
	m_response = "";
} // prepForCall


RESTTimings::RESTTimings(RESTClient* client) {
	this->client = client;
}


/**
 * @brief Refresh the timings information.
 */
void RESTTimings::refresh() {
	::curl_easy_getinfo(client->m_curlHandle, CURLINFO_STARTTRANSFER_TIME, &m_starttransfer);
	::curl_easy_getinfo(client->m_curlHandle, CURLINFO_NAMELOOKUP_TIME, &m_namelookup);
	::curl_easy_getinfo(client->m_curlHandle, CURLINFO_CONNECT_TIME, &m_connect);
	::curl_easy_getinfo(client->m_curlHandle, CURLINFO_APPCONNECT_TIME, &m_appconnect);
	::curl_easy_getinfo(client->m_curlHandle, CURLINFO_PRETRANSFER_TIME, &m_pretransfer);
	::curl_easy_getinfo(client->m_curlHandle, CURLINFO_TOTAL_TIME, &m_total);
} // refresh


/**
 * @brief Return the timings information as a string.
 *
 * @return The timings information.
 */
std::string RESTTimings::toString() {
	std::string ret = "Start Transfer: " + std::to_string(m_starttransfer) + \
			"\nName lookup: " + std::to_string(m_namelookup) + \
			"\nConnect: " + std::to_string(m_connect) + \
			"\nApp Connect: " + std::to_string(m_appconnect) + \
			"\nPre Transfer: " + std::to_string(m_pretransfer) + \
			"\nTotal: " + std::to_string(m_total);
	return ret;
} // toString
#endif // CONFIG_LIBCURL_PRESENT
