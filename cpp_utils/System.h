/*
 * System.h
 *
 *  Created on: May 27, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_SYSTEM_H_
#define COMPONENTS_CPP_UTILS_SYSTEM_H_
#include <stdint.h>

/**
 * @brief System wide functions.
 */
class System {
public:
	System();
	virtual ~System();
	static uint32_t getFreeHeapSize();
};

#endif /* COMPONENTS_CPP_UTILS_SYSTEM_H_ */
