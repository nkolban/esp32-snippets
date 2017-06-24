/*
 * BLEAdvertising.cpp
 *
 *  Created on: Jun 21, 2017
 *      Author: kolban
 */

#include "BLEAdvertising.h"
#include <esp_log.h>
#include <esp_err.h>

static char TAG[] = "BLEAdvertising";

extern "C" {
	char *espToString(esp_err_t value);
}

BLEAdvertising::BLEAdvertising() {
	// TODO Auto-generated constructor stub
	m_advData.set_scan_rsp        = false;
	m_advData.include_name        = true;
	m_advData.include_txpower     = true;
	m_advData.min_interval        = 0x20;
	m_advData.max_interval        = 0x40;
	m_advData.appearance          = 0x00;
	m_advData.manufacturer_len    = 0;
	m_advData.p_manufacturer_data = nullptr;
	m_advData.service_data_len    = 0;
	m_advData.p_service_data      = nullptr;
	m_advData.service_uuid_len    = 0;
	m_advData.p_service_uuid      = nullptr;
	m_advData.flag                = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT);

	m_advParams.adv_int_min       = 0x20;
	m_advParams.adv_int_max       = 0x40;
	m_advParams.adv_type          = ADV_TYPE_IND;
	m_advParams.own_addr_type     = BLE_ADDR_TYPE_PUBLIC;
	m_advParams.channel_map       = ADV_CHNL_ALL;
	m_advParams.adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY;
} // BLEAdvertising

BLEAdvertising::~BLEAdvertising() {
	// TODO Auto-generated destructor stub
}


/**
 * @brief Start advertising.
 * Start advertising.
 * @return N/A.
 */
void BLEAdvertising::start() {
	// Set the configuration for advertising.
	esp_err_t errRc = ::esp_ble_gap_config_adv_data(&m_advData);
	if (errRc != ESP_OK) {
		ESP_LOGE(TAG, "esp_ble_gap_config_adv_data: rc=%d %s", errRc, espToString(errRc));
		return;
	}

	// Start advertising.
	errRc = ::esp_ble_gap_start_advertising(&m_advParams);
	if (errRc != ESP_OK) {
		ESP_LOGE(TAG, "esp_ble_gap_start_advertising: rc=%d %s", errRc, espToString(errRc));
		return;
	}
} // advertising


/**
 * @brief Stop advertising.
 * Stop advertising.
 * @return N/A.
 */
void BLEAdvertising::stop() {
	esp_err_t errRc = ::esp_ble_gap_stop_advertising();
	if (errRc != ESP_OK) {
		ESP_LOGE(TAG, "esp_ble_gap_stop_advertising: rc=%d %s", errRc, espToString(errRc));
		return;
	}
} // stop


/**
 * @brief Set the device appearance in the advertising data.
 * The apperance attribute is of type 0x19.  The codes for distinct appearances can be found here:
 * https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.gap.appearance.xml.
 * @param [in] appearance The appearance of the device in the advertising data.
 * @return N/A.
 */
void BLEAdvertising::setAppearance(uint16_t appearance) {
	m_advData.appearance = appearance;
} // setAppearance
