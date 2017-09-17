/*
 * SSLUtils.h
 *
 *  Created on: Sep 16, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_SSLUTILS_H_
#define COMPONENTS_CPP_UTILS_SSLUTILS_H_
#include <string>
class SSLUtils {
private:
	static char* m_certificate;
	static char* m_key;
public:
	SSLUtils();
	virtual ~SSLUtils();
	static void setCertificate(std::string certificate);
	static char* getCertificate();
	static void setKey(std::string key);
	static char* getKey();
};

#endif /* COMPONENTS_CPP_UTILS_SSLUTILS_H_ */
