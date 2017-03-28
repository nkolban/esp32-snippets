/*
 * NVS.cpp
 *
 *  Created on: Mar 27, 2017
 *      Author: kolban
 */

#include <stdlib.h>
#include "NVS.h"

NVS::NVS(std::string name, nvs_open_mode openMode) {
	m_name = name;
	nvs_open(name.c_str(), openMode, &m_handle);
}


NVS::~NVS() {
	nvs_close(m_handle);
}


void NVS::commit() {
	nvs_commit(m_handle);
}


void NVS::erase() {
	nvs_erase_all(m_handle);
}


void NVS::erase(std::string key) {
	nvs_erase_key(m_handle, key.c_str());
}


void NVS::get(std::string key, std::string* result) {
	size_t length;
	nvs_get_str(m_handle, key.c_str(), NULL, &length);
	char *data = (char *)malloc(length);
	nvs_get_str(m_handle, key.c_str(), data, &length);
	*result = std::string(data);
	free(data);
}


void NVS::set(std::string key, std::string data) {
	nvs_set_str(m_handle, key.c_str(), data.c_str());
}
