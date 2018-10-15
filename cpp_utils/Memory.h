/*
 * Memory.h
 *
 *  Created on: Oct 24, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_MEMORY_H_
#define COMPONENTS_CPP_UTILS_MEMORY_H_
#include "sdkconfig.h"
#ifdef CONFIG_HEAP_TRACING
#include <string>
extern "C" {
#include <esp_heap_trace.h>
}

class Memory {
public:
	static bool checkIntegrity();
	static void dump();
	static void dumpRanges();
	static void dumpHeapChange(std::string tag);
	static void init(uint32_t recordCount);
	static void resumeTrace();
	static void startTraceAll();
	static void startTraceLeaks();
	static void stopTrace();

private:
	static heap_trace_record_t* m_pRecords;
	static size_t               m_lastHeapSize; // Size of last heap recorded.
};
#endif
#endif /* COMPONENTS_CPP_UTILS_MEMORY_H_ */
