/*
 * IFTTT.h
 *
 *  Created on: Mar 14, 2017
 *      Author: kolban
 */

#ifndef MAIN_IFTTT_H_
#define MAIN_IFTTT_H_

#include <RESTClient.h>
#include <string>
/**
 * @brief Encapsulate %IFTTT calls.
 */
class IFTTT {
public:
	IFTTT(std::string key);
	virtual ~IFTTT();
	void trigger(std::string event, std::string value1 = "", std::string value2 = "", std::string value3 = "");
private:
	RESTClient m_restClient;
	std::string m_key;
};

#endif /* MAIN_IFTTT_H_ */
