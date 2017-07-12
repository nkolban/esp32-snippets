/*
 * NVS.cpp
 *
 *  Created on: Mar 27, 2017
 *      Author: kolban
 */

#include "CPPNVS.h"

#include <stdlib.h>

/**
 * @brief Constructor.
 *
 * @param [in] name The namespace to open for access.
 * @param [in] openMode
 */
NVS::NVS(std::string name, nvs_open_mode openMode) {
	m_name = name;
	nvs_open(name.c_str(), openMode, &m_handle);
} // NVS


NVS::~NVS() {
	nvs_close(m_handle);
} // ~NVS


/**
 * @brief Commit any work performed in the namespace.
 */
void NVS::commit() {
	nvs_commit(m_handle);
} // commit


/**
 * @brief Erase ALL the keys in the namespace.
 */
void NVS::erase() {
	nvs_erase_all(m_handle);
} // erase


/**
 * @brief Erase a specific key in the namespace.
 *
 * @param [in] key The key to erase from the namespace.
 */
void NVS::erase(std::string key) {
	nvs_erase_key(m_handle, key.c_str());
} // erase


/**
 * @brief Retrieve a string value by key.
 *
 * @param [in] key The key to read from the namespace.
 * @param [out] result The string read from the %NVS storage.
 */
void NVS::get(std::string key, std::string* result) {
	size_t length;
	nvs_get_str(m_handle, key.c_str(), NULL, &length);
	char *data = (char *)malloc(length);
	nvs_get_str(m_handle, key.c_str(), data, &length);
	*result = std::string(data);
	free(data);
} // get


/**
 * @brief Set the string value by key.
 *
 * @param [in] key The key to set from the namespace.
 * @param [in] data The value to set for the key.
 */
void NVS::set(std::string key, std::string data) {
	nvs_set_str(m_handle, key.c_str(), data.c_str());
} // set
