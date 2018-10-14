/*
 * IFTTT.cpp
 *
 *  Created on: Mar 14, 2017
 *      Author: kolban
 */
#if defined(ESP_HAVE_CURL)
#include "IFTTT.h"
#include <cJSON.h>


/**
 * @brief Construct an IFTTT maker client using the supplied key.
 */
IFTTT::IFTTT(std::string key) {
	m_key = key;
	m_restClient.addHeader("Content-Type", "application/json");
	m_restClient.setVerbose(true);
} // IFTTT


IFTTT::~IFTTT() {
} // ~IFTTT


/**
 * @brief Trigger a maker event at IFTTT.
 *
 * @param [in] event The event type to send.
 * @param [in] value1 The value of value1.
 * @param [in] value2 The value of value2.
 * @param [in] value3 The value of value3.
 */
void IFTTT::trigger(
		std::string event,
		std::string value1,
		std::string value2,
		std::string value3) {
	m_restClient.setURL("https://maker.ifttt.com/trigger/" + event + "/with/key/" + m_key);
	cJSON* root;
	root = cJSON_CreateObject();

	cJSON_AddStringToObject(root, "value1", value1.c_str());
	cJSON_AddStringToObject(root, "value2", value2.c_str());
	cJSON_AddStringToObject(root, "value3", value3.c_str());

	m_restClient.post(std::string(cJSON_Print(root)));

	cJSON_Delete(root);
} // trigger
#endif // ESP_HAVE_CURL
