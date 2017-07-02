/*
 * BLEScan.cpp
 *
 *  Created on: Jul 1, 2017
 *      Author: kolban
 */

#include "BLEScan.h"
#include <esp_log.h>
#include <esp_err.h>

static char LOG_TAG[] = "BLEScan";


BLEScan::BLEScan() {
	m_scan_params.scan_type          = BLE_SCAN_TYPE_PASSIVE;
	m_scan_params.own_addr_type      = BLE_ADDR_TYPE_PUBLIC;
	m_scan_params.scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL;
	setInterval(100);
	setWindow(100);
}


BLEScan::~BLEScan() {
}


void BLEScan::onResults() {
	ESP_LOGD(LOG_TAG, ">> onResults: default");
	ESP_LOGD(LOG_TAG, "<< onResults");
} // onResults


/**
 * @brief Should we perform an active or passive scan?
 * The default is a passive scan.
 * @param [in] active If true, we perform an active scan otherwise a passive scan.
 * @return N/A.
 */
void BLEScan::setActiveScan(bool active) {
	if (active) {
		m_scan_params.scan_type = BLE_SCAN_TYPE_ACTIVE;
	} else {
		m_scan_params.scan_type = BLE_SCAN_TYPE_PASSIVE;
	}
} // setActiveScan

void BLEScan::setInterval(uint16_t intervalMSecs) {
	m_scan_params.scan_interval = intervalMSecs / 0.625;
} // setInterval

void BLEScan::setWindow(uint16_t windowMSecs) {
	m_scan_params.scan_window = windowMSecs / 0.625;
} // setWindow

void BLEScan::start(uint32_t duration) {
	esp_err_t errRc = ::esp_ble_gap_set_scan_params(&m_scan_params);
	errRc = ::esp_ble_gap_start_scanning(duration);
} // start

void BLEScan::stop() {
	esp_err_t errRc = ::esp_ble_gap_stop_scanning();
} // stop
