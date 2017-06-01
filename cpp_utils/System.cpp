/*
 * System.cpp
 *
 *  Created on: May 27, 2017
 *      Author: kolban
 */

#include "System.h"
#include <esp_system.h>

System::System() {
	// TODO Auto-generated constructor stub

}

System::~System() {
	// TODO Auto-generated destructor stub
}

/**
 * @brief Retrieve the system wide free heap size.
 * @return The system wide free heap size.
 */
uint32_t System::getFreeHeapSize() {
	return esp_get_free_heap_size();
} // getFreeHeapSize
