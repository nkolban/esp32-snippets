/*
 * System.cpp
 *
 *  Created on: May 27, 2017
 *      Author: kolban
 */

#include "System.h"
#include <esp_system.h>

extern "C" {
#include <esp_heap_caps.h>
}

System::System() {
	// TODO Auto-generated constructor stub

}

System::~System() {
	// TODO Auto-generated destructor stub
}

/**
 * @brief Get the information about the device.
 * @param [out] info The structure to be populated on return.
 * @return N/A.
 */
void System::getChipInfo(esp_chip_info_t *info) {
	::esp_chip_info(info);
} // getChipInfo


/**
 * @brief Retrieve the system wide free heap size.
 * @return The system wide free heap size.
 */
size_t System::getFreeHeapSize() {
	return heap_caps_get_free_size(MALLOC_CAP_8BIT);
} // getFreeHeapSize


/**
 * @brief Retrieve the version of the ESP-IDF.
 * When an application is compiled, it is compiled against a version of the ESP-IDF.
 * This function returns that version.
 */
std::string System::getIDFVersion() {
	return std::string(::esp_get_idf_version());
} // getIDFVersion


/**
 * @brief Get the smallest heap size seen.
 * @return The smallest heap size seen.
 */
size_t System::getMinimumFreeHeapSize() {
	return heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);
} // getMinimumFreeHeapSize

/**
 * @brief Restart the ESP32.
 */
void System::restart() {
	esp_restart();
} // restart
