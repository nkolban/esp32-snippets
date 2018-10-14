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
	static void dumpPinMapping();                       // Dump the mappings of pins to functions.
	static void dumpHeapInfo();
	static void getChipInfo(esp_chip_info_t* info);
	static size_t getFreeHeapSize();
	static std::string getIDFVersion();
	static size_t getMinimumFreeHeapSize();
	static void restart();
};

#endif /* COMPONENTS_CPP_UTILS_SYSTEM_H_ */
