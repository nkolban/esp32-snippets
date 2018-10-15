/*
 * Memory.cpp
 *
 *  Created on: Oct 24, 2017
 *      Author: kolban
 */
#include "sdkconfig.h"
#ifdef CONFIG_HEAP_TRACING
#include "Memory.h"

#include <stdlib.h>
#include "GeneralUtils.h"
extern "C" {
	#include <esp_heap_trace.h>
	#include <esp_heap_caps.h>
}
#include <esp_log.h>
#include <esp_err.h>

static const char* LOG_TAG = "Memory";

heap_trace_record_t* Memory::m_pRecords = nullptr;
size_t               Memory::m_lastHeapSize = 0;


/**
 * @brief Check the integrity of memory.
 * @return True if all integrity checks passed
 */
/* STATIC */ bool Memory::checkIntegrity() {
	bool rc = ::heap_caps_check_integrity_all(true);
	if (!rc && m_pRecords != nullptr) {
		dumpRanges();
		abort();
	}
	return rc;
} // checkIntegrity


/**
 * @brief Dump the trace records from the heap.
 */
/* STATIC */ void Memory::dump() {
	::heap_trace_dump();
} // dump


/* STATIC */ void Memory::dumpHeapChange(std::string tag) {
	size_t currentUsage = heap_caps_get_free_size(MALLOC_CAP_8BIT);
	size_t diff = currentUsage - m_lastHeapSize;
	ESP_LOGD(LOG_TAG, "%s: Heap changed by %d bytes  (%d to %d)", tag.c_str(), diff, m_lastHeapSize, currentUsage);
	m_lastHeapSize = currentUsage;
} // dumpHeapChange


/**
 * @brief Dump the ranges of allocations found.
 */
/* STATIC */ void Memory::dumpRanges() {
	// Each record contained in the Heap trace has the following format:
	// * uint32_t ccount – Timestamp of record.
	// * void*    address – Address that was allocated or released.
	// * size_t   size – Size of the block that was requested and allocated.
	// * void*    alloced_by[CONFIG_HEAP_TRACING_STACK_DEPTH] – Call stack of allocator
	// * void*    freed_by[CONFIG_HEAP_TRACING_STACK_DEPTH] – Call stack of releasor
	if (m_pRecords == nullptr) return;

	esp_log_level_set("*", ESP_LOG_NONE);
	size_t count = (size_t) heap_trace_get_count();
	heap_trace_record_t record;
	printf(">>> dumpRanges\n");
	for (size_t i = 0; i < count; i++) {
		esp_err_t errRc = heap_trace_get(i, &record);
		if (errRc != ESP_OK) {
			ESP_LOGE(LOG_TAG, "heap_trace_get: %d", errRc);
		}
		printf("0x%x:0x%x:%d:", (uint32_t) record.address, ((uint32_t) record.address) + record.size, record.size);
		for (size_t j = 0; j < CONFIG_HEAP_TRACING_STACK_DEPTH; j++) {
			printf("%x ", (uint32_t) record.alloced_by[j]);
		}
		printf(":");
		for (size_t j = 0; j < CONFIG_HEAP_TRACING_STACK_DEPTH; j++) {
			printf("%x ", (uint32_t) record.freed_by[j]);
		}
		printf("\n");
	}
	printf("<<< dumpRanges\n");
	esp_log_level_set("*", ESP_LOG_VERBOSE);
} // dumpRanges


/**
 * @brief Initialize heap recording.
 * @param [in] recordCount The maximum number of records to be recorded.
 */
/* STATIC */ void Memory::init(uint32_t recordCount) {
	assert(recordCount > 0);
	if (m_pRecords != nullptr) {
		ESP_LOGE(LOG_TAG, "Already initialized");
		return;
	}

	m_pRecords = new heap_trace_record_t[recordCount];   // Allocate the maximum number of records to be recorded.
	if (m_pRecords == nullptr) {
		ESP_LOGE(LOG_TAG, "Unable to create %d heap records", recordCount);
	}

	esp_err_t errRc = ::heap_trace_init_standalone(m_pRecords, recordCount);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "heap_trace_init_standalone: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		abort();
	}
} // init


/**
 * @brief Resume previously paused trace.
 */
/* STATIC */ void Memory::resumeTrace() {
	esp_err_t errRc = ::heap_trace_resume();
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "heap_trace_resume: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		abort();
	}
} // resumeTrace


/**
 * @brief Start tracing all allocate and free calls.
 */
/* STATIC */ void Memory::startTraceAll() {
	esp_err_t errRc = ::heap_trace_start(HEAP_TRACE_ALL);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "heap_trace_start: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		abort();
	}
} // startTraceAll


/**
 * Start tracing leaks.  Matched allocate and free calls are removed.
 */
/* STATIC */ void Memory::startTraceLeaks() {
	esp_err_t errRc = ::heap_trace_start(HEAP_TRACE_LEAKS);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "heap_trace_start: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		abort();
	}
} // startTraceLeaks


/**
 * @brief Stop recording heap trace.
 */
/* STATIC */ void Memory::stopTrace() {
	esp_err_t errRc = ::heap_trace_stop();
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "heap_trace_start: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		abort();
	}
} // stopTrace

#endif
