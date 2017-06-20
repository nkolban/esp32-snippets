/*
 * System.h
 *
 *  Created on: May 27, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_SYSTEM_H_
#define COMPONENTS_CPP_UTILS_SYSTEM_H_
#include <stdint.h>
#include <string>
#include <esp_system.h>

/**
 * @brief System wide functions.
 */
class System {
public:
	System();
	virtual ~System();
	static void getChipInfo(esp_chip_info_t *info);
	static uint32_t getFreeHeapSize();
	static std::string getIDFVersion();
};

#endif /* COMPONENTS_CPP_UTILS_SYSTEM_H_ */
