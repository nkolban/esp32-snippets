/*
 * NVS.h
 *
 *  Created on: Mar 27, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_CPPNVS_H_
#define COMPONENTS_CPP_UTILS_CPPNVS_H_
#include <nvs.h>
#include <string>

/**
 * @brief Provide Non Volatile Storage access.
 */
class NVS {
public:
	NVS(std::string name, nvs_open_mode openMode = NVS_READWRITE);
	virtual ~NVS();
	void commit();

	void erase();
	void erase(std::string key);
	int get(std::string key, std::string* result, bool isBlob = false);
	int get(std::string key, uint8_t* result, size_t& length);
	int get(std::string key, uint32_t& value);
	void set(std::string key, std::string data, bool isBlob = false);
	void set(std::string key, uint32_t value);
	void set(std::string key, uint8_t* data, size_t length);

private:
	std::string m_name;
	nvs_handle m_handle;

};

#endif /* COMPONENTS_CPP_UTILS_CPPNVS_H_ */
