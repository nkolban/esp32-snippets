/*
 * BLEAdvertising.cpp
 *
 * This class encapsulates advertising a BLE Server.
 *  Created on: Jun 21, 2017
 *      Author: kolban
 */
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include "BLEAdvertising.h"
#include <esp_log.h>
#include <esp_err.h>
#include "BLEUtils.h"
#include "GeneralUtils.h"

static char LOG_TAG[] = "BLEAdvertising";


/**
 * @brief Construct a default advertising object.
 *
 */
BLEAdvertising::BLEAdvertising() {
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
} // ~BLEAdvertising



/**
 * @brief Set the device appearance in the advertising data.
 * The appearance attribute is of type 0x19.  The codes for distinct appearances can be found here:
 * https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.gap.appearance.xml.
 * @param [in] appearance The appearance of the device in the advertising data.
 * @return N/A.
 */
void BLEAdvertising::setAppearance(uint16_t appearance) {
	m_advData.appearance = appearance;
} // setAppearance


/**
 * @brief Set the service UUID.
 * @param [in] uuid The UUID of the service.
 * @return N/A.
 */
void BLEAdvertising::setServiceUUID(BLEUUID uuid) {
	ESP_LOGD(LOG_TAG, ">> setServiceUUID(%s)", uuid.toString().c_str());
	m_serviceUUID = uuid; // Save the new service UUID
	esp_bt_uuid_t espUUID = *m_serviceUUID.getNative();
	switch(espUUID.len) {
		case ESP_UUID_LEN_16: {
			m_advData.service_uuid_len = 2;
			m_advData.p_service_uuid = reinterpret_cast<uint8_t*>(&espUUID.uuid.uuid16);
			break;
		}
		case ESP_UUID_LEN_32: {
			m_advData.service_uuid_len = 4;
			m_advData.p_service_uuid = reinterpret_cast<uint8_t*>(&espUUID.uuid.uuid32);
			break;
		}
		case ESP_UUID_LEN_128: {
			m_advData.service_uuid_len = 16;
			m_advData.p_service_uuid = reinterpret_cast<uint8_t*>(&espUUID.uuid.uuid128);
			break;
		}
	} // switch
	ESP_LOGD(LOG_TAG, "<< setServiceUUID()");
} // setServiceUUID


/**
 * @brief Start advertising.
 * Start advertising.
 * @return N/A.
 */
void BLEAdvertising::start() {
	ESP_LOGD(LOG_TAG, ">> start()");

	if (m_advData.service_uuid_len > 0) {
		uint8_t hexData[16*2+1];
		BLEUtils::buildHexData(hexData, m_advData.p_service_uuid, m_advData.service_uuid_len);
		ESP_LOGD(LOG_TAG, " - Service: service_uuid_len=%d, p_service_uuid=0x%x (data=%s)",
			m_advData.service_uuid_len,
			(uint32_t)m_advData.p_service_uuid,
			(m_advData.service_uuid_len > 0?(char *)hexData:"N/A")
		);
	} // We have a service to advertise


	// Set the configuration for advertising.
	esp_err_t errRc = ::esp_ble_gap_config_adv_data(&m_advData);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "<< esp_ble_gap_config_adv_data: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}

	// Start advertising.
	errRc = ::esp_ble_gap_start_advertising(&m_advParams);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "<< esp_ble_gap_start_advertising: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}
	ESP_LOGD(LOG_TAG, "<< start();")
} // start


/**
 * @brief Stop advertising.
 * Stop advertising.
 * @return N/A.
 */
void BLEAdvertising::stop() {
	esp_err_t errRc = ::esp_ble_gap_stop_advertising();
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gap_stop_advertising: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}
} // stop
#endif /* CONFIG_BT_ENABLED */
