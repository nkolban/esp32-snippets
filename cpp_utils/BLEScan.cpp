/*
 * BLEScan.cpp
 *
 *  Created on: Jul 1, 2017
 *      Author: kolban
 */
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)


#include <esp_log.h>
#include <esp_err.h>

#include <map>

#include "BLEAdvertisedDevice.h"
#include "BLEScan.h"
#include "BLEUtils.h"
#include "GeneralUtils.h"

static char LOG_TAG[] = "BLEScan";



BLEScan::BLEScan() {
	m_scan_params.scan_type          = BLE_SCAN_TYPE_PASSIVE; // Default is a passive scan.
	m_scan_params.own_addr_type      = BLE_ADDR_TYPE_PUBLIC;
	m_scan_params.scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL;
	setInterval(100);
	setWindow(100);
	m_pAdvertisedDeviceCallbacks = nullptr;
	m_stopped = true;
}


BLEScan::~BLEScan() {
	clearAdvertisedDevices();
}

void BLEScan::clearAdvertisedDevices() {
	for (int i=0; i<m_vectorAvdertisedDevices.size(); i++) {
		delete m_vectorAvdertisedDevices[i];
	}
	m_vectorAvdertisedDevices.clear();
}


/**
 * @brief Handle GAP events related to scans.
 * @param [in] event The event type for this event.
 * @param [in] param Parameter data for this event.
 */
void BLEScan::gapEventHandler(
	esp_gap_ble_cb_event_t event,
	esp_ble_gap_cb_param_t *param) {
	switch(event) {

	// ESP_GAP_BLE_SCAN_RESULT_EVT
	// ---------------------------
	// scan_rst:
	// esp_gap_search_evt_t search_evt
	// esp_bd_addr_t bda
	// esp_bt_dev_type_t dev_type
	// esp_ble_addr_type_t ble_addr_type
	// esp_ble_evt_type_t ble_evt_type
	// int rssi
	// uint8_t ble_adv[ESP_BLE_ADV_DATA_LEN_MAX]
	// int flag
	// int num_resps
	// uint8_t adv_data_len
	// uint8_t scan_rsp_len
		case ESP_GAP_BLE_SCAN_RESULT_EVT: {

			switch(param->scan_rst.search_evt) {
				case ESP_GAP_SEARCH_INQ_CMPL_EVT: {
					m_stopped = true;
					m_semaphoreScanEnd.give();
					break;
				}

				case ESP_GAP_SEARCH_INQ_RES_EVT: {
					if (m_stopped) { // If we are not scanning, nothing to do with the extra results.
						break;
					}

// Examine our list of previously scanned addresses and, if we found this one already,
// ignore it.
					BLEAddress advertisedAddress(param->scan_rst.bda);
					bool found = false;
					for (int i=0; i<m_vectorAvdertisedDevices.size(); i++) {
						if (m_vectorAvdertisedDevices[i]->getAddress().equals(advertisedAddress)) {
							found = true;
							break;
						}
					}
					if (found) {
						ESP_LOGD(LOG_TAG, "Ignoring %s, already seen it.", advertisedAddress.toString().c_str());
						break;
					}

					BLEAdvertisedDevice* pAdvertisedDevice = new BLEAdvertisedDevice();
					pAdvertisedDevice->setAddress(advertisedAddress);
					pAdvertisedDevice->setRSSI(param->scan_rst.rssi);
					pAdvertisedDevice->setAdFlag(param->scan_rst.flag);
					pAdvertisedDevice->parseAdvertisement((uint8_t *)param->scan_rst.ble_adv);
					pAdvertisedDevice->setScan(this);
					m_vectorAvdertisedDevices.push_back(pAdvertisedDevice);
					if (m_pAdvertisedDeviceCallbacks) {
						m_pAdvertisedDeviceCallbacks->onResult(pAdvertisedDevice);
					}

					break;
				}

				default: {
					break;
				}
			} // switch - search_evt


			break;
		} // ESP_GAP_BLE_SCAN_RESULT_EVT

		default: {
			break;
		} // default
	} // End switch
} // gapEventHandler

void BLEScan::onResults() {
	ESP_LOGD(LOG_TAG, ">> onResults: default");
	ESP_LOGD(LOG_TAG, "<< onResults");
} // onResults


/**
 * @brief Should we perform an active or passive scan?
 * The default is a passive scan.  An active scan means that we will wish a scan response.
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


/**
 * @brief Set the callbacks to be invoked.
 * @param [in] pAdvertisedDeviceCallbacks Callbacks to be invoked.
 */
void BLEScan::setAdvertisedDeviceCallbacks(BLEAdvertisedDeviceCallbacks* pAdvertisedDeviceCallbacks) {
	m_pAdvertisedDeviceCallbacks = pAdvertisedDeviceCallbacks;
} // setAdvertisedDeviceCallbacks


/**
 * @brief Set the interval to scan.
 * @param [in] The interval in msecs.
 */
void BLEScan::setInterval(uint16_t intervalMSecs) {
	m_scan_params.scan_interval = intervalMSecs / 0.625;
} // setInterval


/**
 * @brief Set the window to actively scan.
 * @param [in] windowMSecs How long to actively scan.
 */
void BLEScan::setWindow(uint16_t windowMSecs) {
	m_scan_params.scan_window = windowMSecs / 0.625;
} // setWindow


/**
 * @brief Start scanning.
 * @param [in] duration The duration in seconds for which to scan.
 * @return N/A.
 */
std::vector<BLEAdvertisedDevice*> BLEScan::start(uint32_t duration) {
	ESP_LOGD(LOG_TAG, ">> start(%d)", duration);
	m_semaphoreScanEnd.take("start");
	clearAdvertisedDevices();
	esp_err_t errRc = ::esp_ble_gap_set_scan_params(&m_scan_params);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gap_set_scan_params: err: %d, text: %s", errRc, GeneralUtils::errorToString(errRc));
		m_semaphoreScanEnd.give();
		return m_vectorAvdertisedDevices;
	}
	errRc = ::esp_ble_gap_start_scanning(duration);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gap_start_scanning: err: %d, text: %s", errRc, GeneralUtils::errorToString(errRc));
		m_semaphoreScanEnd.give();
		return m_vectorAvdertisedDevices;
	}
	m_stopped = false;
	m_semaphoreScanEnd.take("start");
	m_semaphoreScanEnd.give();
	ESP_LOGD(LOG_TAG, "<< start()");
	return m_vectorAvdertisedDevices;
} // start


/**
 * @brief Stop an in progress scan.
 * @return N/A.
 */
void BLEScan::stop() {
	ESP_LOGD(LOG_TAG, ">> stop()");
	m_stopped = true;
	esp_err_t errRc = ::esp_ble_gap_stop_scanning();
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gap_stop_scanning: err: %d, text: %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}
	m_semaphoreScanEnd.give();
	ESP_LOGD(LOG_TAG, "<< stop()");
} // stop



#endif /* CONFIG_BT_ENABLED */
