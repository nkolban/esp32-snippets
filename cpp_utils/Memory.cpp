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

static const char* LOG_TAG = "Memory";

heap_trace_record_t* Memory::m_pRecords = nullptr;
size_t               Memory::m_lastHeapSize = 0;

/**
 * @brief Dump the trace records from the heap.
 */
void Memory::dump() {
	::heap_trace_dump();
} // dump


/* STATIC */ void Memory::dumpHeapChange(std::string tag) {
	size_t currentUsage = heap_caps_get_free_size(MALLOC_CAP_8BIT);
	int diff = currentUsage - m_lastHeapSize;
	ESP_LOGD(LOG_TAG, "%s: Heap changed by %d bytes  (%d to %d)", tag.c_str(), diff, m_lastHeapSize, currentUsage);
	m_lastHeapSize = currentUsage;
}

/**
 * @brief Initialize heap recording.
 * @param [in] recordCount The maximum number of records to be recorded.
 */
void Memory::init(uint32_t recordCount) {
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


void Memory::resumeTrace() {
	esp_err_t errRc = ::heap_trace_resume();
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "heap_trace_resume: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		abort();
	}
} // resumeTrace


void Memory::startTraceAll() {
	esp_err_t errRc = ::heap_trace_start(HEAP_TRACE_ALL);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "heap_trace_start: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		abort();
	}
} // startTraceAll


void Memory::startTraceLeaks() {
	esp_err_t errRc = ::heap_trace_start(HEAP_TRACE_LEAKS);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "heap_trace_start: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		abort();
	}
} // startTraceLeaks


void Memory::stopTrace() {
	esp_err_t errRc = ::heap_trace_stop();
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "heap_trace_start: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		abort();
	}
} // stopTrace

#endif
