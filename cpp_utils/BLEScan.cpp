/*
 * BLEScan.cpp
 *
 *  Created on: Jul 1, 2017
 *      Author: kolban
 */

#include "BLERemoteDevice.h"
#include "BLEScan.h"
#include "BLEUtils.h"
#include "GeneralUtils.h"

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
			BLERemoteDevice *pDevice = new BLERemoteDevice();
			switch(param->scan_rst.search_evt) {
				case ESP_GAP_SEARCH_INQ_CMPL_EVT: {
					break;
				}

				case ESP_GAP_SEARCH_INQ_RES_EVT: {
					pDevice->setAddress(BLEAddress(param->scan_rst.bda));
					pDevice->setRSSI(param->scan_rst.rssi);
					pDevice->setAdFlag(param->scan_rst.flag);

					parseAdvertisement(pDevice, (uint8_t *)param->scan_rst.ble_adv);
					break;
				}

				default: {
					break;
				}
			} // switch - search_evt
			delete pDevice;
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
void BLEScan::start(uint32_t duration) {
	esp_err_t errRc = ::esp_ble_gap_set_scan_params(&m_scan_params);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gap_set_scan_params: err: %d, text: %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}
	errRc = ::esp_ble_gap_start_scanning(duration);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gap_start_scanning: err: %d, text: %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}
} // start


/**
 * @brief Stop an in progress scan.
 * @return N/A.
 */
void BLEScan::stop() {
	esp_err_t errRc = ::esp_ble_gap_stop_scanning();
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gap_stop_scanning: err: %d, text: %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}
} // stop


/**
 * @brief Parse the advertising pay load.
 *
 * The pay load is a buffer of bytes that is either 31 bytes long or terminated by
 * a 0 length value.  Each entry in the buffer has the format:
 * [length][type][data...]
 *
 * The length does not include itself but does include everything after it until the next record.  A record
 * with a length value of 0 indicates a terminator.
 *
 * https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile
 */
void BLEScan::parseAdvertisement(BLERemoteDevice *pRemoteDevice, uint8_t *payload) {
	uint8_t length;
	uint8_t ad_type;
	uint8_t sizeConsumed = 0;
	bool finished = false;

	while(!finished) {
		length = *payload; // Retrieve the length of the record.
		payload++; // Skip to type
		sizeConsumed += 1 + length; // increase the size consumed.

		if (length != 0) { // A length of 0 indicate that we have reached the end.
			ad_type = *payload;
			payload++;

			ESP_LOGD(LOG_TAG, "Type: 0x%.2x (%s), length: %d", ad_type, BLEUtils::advTypeToString(ad_type), length);

			length--;

			switch(ad_type) {
				case ESP_BLE_AD_TYPE_NAME_CMPL: {
					pRemoteDevice->setName(std::string((char *)payload, length));
					break;
				}

				case ESP_BLE_AD_TYPE_TX_PWR: {
					pRemoteDevice->setTXPower(*payload);
					break;
				}

				case ESP_BLE_AD_TYPE_APPEARANCE: {
					pRemoteDevice->setAppearance(*(uint16_t *)payload);
					break;
				}

				case ESP_BLE_AD_TYPE_FLAG: {
					pRemoteDevice->setAdFlag((uint8_t)*payload);
					break;
				}

				/*
				case ESP_BLE_AD_TYPE_16SRV_PART:
					//m_services.insert(std::string((char *) payload, 2));
					break;

				case ESP_BLE_AD_TYPE_128SRV_PART:
					//m_services.insert(std::string((char *) payload, 16));
					break;



				case ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE:
					assert(length >=2);
					//m_manufacturerType[0] = payload[0];
					//m_manufacturerType[1] = payload[1];
					break;
					*/

				default:
					ESP_LOGD(LOG_TAG, "Unhandled type");
					break;
			} // switch
			payload += length;
		} // Length <> 0


		if (sizeConsumed >=31 || length == 0) {
			finished = true;
		}
	} // !finished
} // dump_adv_payload
