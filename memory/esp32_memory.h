/*
 * esp32_memory.h
 *
 *  Created on: Dec 9, 2016
 *      Author: kolban
 */

#ifndef MAIN_ESP32_MEMORY_H_
#define MAIN_ESP32_MEMORY_H_

#include <esp_system.h>
#include <esp_log.h>

static uint32_t _heapFreeBefore;
static uint32_t _counter = 0;

#define HEAP_CHANGE_START()  _heapFreeBefore = esp_get_free_heap_size()
#define HEAP_CHANGE_END(_EYECATCHER) { \
	uint32_t newSize = esp_get_free_heap_size(); \
	uint32_t value = _heapFreeBefore - newSize; \
	if (value !=0) { \
		ESP_LOGD("memory", _EYECATCHER ": Heap changed by %d, new size: %d, counter: %d", value, newSize, _counter); \
		_counter = 0; \
	} else { _counter++; }\
}


#endif /* MAIN_ESP32_MEMORY_H_ */
