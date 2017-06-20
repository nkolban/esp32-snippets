/*
 * BLEServer.cpp
 *
 *  Created on: Apr 16, 2017
 *      Author: kolban
 */

#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <esp_log.h>
#include <bt.h>
#include <esp_bt_main.h>
#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>
#include "BLEServer.h"
#include "BLEUtils.h"
#include <string>
#include <gatt_api.h>
#include <unordered_set>

static char tag[] = "BLEServer";

extern "C" {
	char *espToString(esp_err_t value);
}

static esp_attr_value_t gatts_demo_char1_val;
static uint8_t char1_str[] = {0x11,0x22,0x33};

/**
 * Construct a BLE Server
 */
BLEServer::BLEServer(uint16_t appId, std::string deviceName) {
	m_appId = appId;
	m_deviceName = deviceName;
	m_profile.service_id.is_primary = true;
	m_profile.service_id.id.inst_id = 0;
	m_profile.service_id.id.uuid.len = ESP_UUID_LEN_16;
	m_profile.service_id.id.uuid.uuid.uuid16 = 0x1234;

	gatts_demo_char1_val.attr_max_len = 0x40;
	gatts_demo_char1_val.attr_len = sizeof(char1_str);
	gatts_demo_char1_val.attr_value = char1_str;

	uint8_t test_service_uuid128[32] = {
	    /* LSB <--------------------------------------------------------------------------------> MSB */
	    //first uuid, 16bit, [12],[13] is the value
	    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xAB, 0xCD, 0x00, 0x00,
	    //second uuid, 32bit, [12], [13], [14], [15] is the value
	    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xAB, 0xCD, 0xAB, 0xCD,
	};
	setUUID(test_service_uuid128);
	::esp_ble_gatts_app_register(m_appId);
} // BLEServer

BLEServer::~BLEServer() {
}


/**
 * @brief setDeviceName
 * @param [in] deviceName
 */
void BLEServer::setDeviceName(std::string deviceName) {
	m_deviceName = deviceName;
} // setDeviceName


/**
 * @brief setUUID
 * @param [in] uuid
 */
void BLEServer::setUUID(uint8_t uuid[32]) {
	memcpy(m_uuid, uuid, 32);
} // setUUID

/**
 * @brief Handle a GATT Server Event.
 * @param [in] event
 * @param [in] gatts_if
 * @param [in] param
 *
 */
void BLEServer::handleGATTServerEvent(
		esp_gatts_cb_event_t event,
		esp_gatt_if_t gatts_if,
		esp_ble_gatts_cb_param_t *param) {
	ESP_LOGD(tag, "handleGATTServerEvent: BLEServer ... handling GATT Server event!");

	switch(event) {
		// ESP_GATTS_REG_EVT
		case ESP_GATTS_REG_EVT: {
			m_profile.gatts_if = gatts_if;
			ESP_LOGD(tag, "Registering device name: %s", m_deviceName.c_str());
			esp_err_t errRc = esp_ble_gap_set_device_name(m_deviceName.c_str());
			if (errRc != ESP_OK) {
				ESP_LOGE(tag, "esp_ble_gap_set_device_name: rc=%d %s", errRc, espToString(errRc));
				return;
			}

			m_adv_data.set_scan_rsp        = false;
			m_adv_data.include_name        = true;
			m_adv_data.include_txpower     = true;
			m_adv_data.min_interval        = 0x20;
			m_adv_data.max_interval        = 0x40;
			m_adv_data.appearance          = 0x00;
			m_adv_data.manufacturer_len    = 0;
			m_adv_data.p_manufacturer_data = NULL;
			m_adv_data.service_data_len    = 0;
			m_adv_data.p_service_data      = NULL;
			m_adv_data.service_uuid_len    = 32;
			m_adv_data.p_service_uuid      = m_uuid;
			m_adv_data.flag                = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT);
			ESP_LOGD(tag, "Setting advertising data");
			errRc = ::esp_ble_gap_config_adv_data(&m_adv_data);
			if (errRc != ESP_OK) {
				ESP_LOGE(tag, "esp_ble_gap_config_adv_data: rc=%d %s", errRc, espToString(errRc));
				return;
			}

			ESP_LOGD(tag, "Creating service");
			errRc = ::esp_ble_gatts_create_service(gatts_if, &m_profile.service_id, 4);
			if (errRc != ESP_OK) {
				ESP_LOGE(tag, "esp_ble_gatts_create_service: rc=%d %s", errRc, espToString(errRc));
				return;
			}
			break;
		} // ESP_GATTS_REG_EVT

		// ESP_GATTS_CREATE_EVT
		case ESP_GATTS_CREATE_EVT: {
			m_profile.service_handle = param->create.service_handle;
			m_profile.char_uuid.len = ESP_UUID_LEN_16;
			m_profile.char_uuid.uuid.uuid16 = 0x99AA;
			ESP_LOGD(tag, "Starting service");
			esp_err_t errRc = ::esp_ble_gatts_start_service(m_profile.service_handle);
			if (errRc != ESP_OK) {
				ESP_LOGE(tag, "esp_ble_gatts_start_service: rc=%d %s", errRc, espToString(errRc));
				return;
			}
			ESP_LOGD(tag, "Adding characteristic");
      errRc = ::esp_ble_gatts_add_char(
      	m_profile.service_handle,
				&m_profile.char_uuid,
				(esp_gatt_perm_t)(ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE),
				(esp_gatt_char_prop_t)(ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY),
				&gatts_demo_char1_val,
				NULL);
			if (errRc != ESP_OK) {
				ESP_LOGE(tag, "esp_ble_gatts_add_char: rc=%d %s", errRc, espToString(errRc));
				return;
			}
			break;
		} // ESP_GATTS_CREATE_EVT


		// ESP_GATTS_READ_EVT - A request to read the value of a characteristic has arrived.
		case ESP_GATTS_READ_EVT: {
			ESP_LOGD(tag, "Sending a response");
			if (param->read.need_rsp) {
				esp_gatt_rsp_t rsp;
				rsp.attr_value.len = 1;
				rsp.attr_value.handle = param->read.handle;
				rsp.attr_value.offset = 0;
				rsp.attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
				rsp.attr_value.value[0] = 'X';
				esp_err_t errRc = ::esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, &rsp);
				if (errRc != ESP_OK) {
					ESP_LOGE(tag, "esp_ble_gatts_add_char: rc=%d %s", errRc, espToString(errRc));
				}
			}
			break;
		} // ESP_GATTS_READ_EVT


		default:
			break;
	}
} // handleGATTServerEvent


/**
 * @brief handleGAPEvent
 * @param [in] event
 * @param [in] param
 */
void BLEServer::handleGAPEvent(
		esp_gap_ble_cb_event_t event,
		esp_ble_gap_cb_param_t *param) {
	ESP_LOGD(tag, "BLEServer ... handling GAP event!");
	switch(event) {
		case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT: {
			esp_ble_adv_params_t adv_params;
			adv_params.adv_int_min       = 0x20;
			adv_params.adv_int_max       = 0x40;
			adv_params.adv_type          = ADV_TYPE_IND;
			adv_params.own_addr_type     = BLE_ADDR_TYPE_PUBLIC;
			adv_params.channel_map       = ADV_CHNL_ALL;
			adv_params.adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY;
			ESP_LOGD(tag, "Starting advertising");
			esp_err_t errRc = ::esp_ble_gap_start_advertising(&adv_params);
			if (errRc != ESP_OK) {
				ESP_LOGE(tag, "esp_ble_gap_start_advertising: rc=%d %s", errRc, espToString(errRc));
				return;
			}
			break;
		}
		default:
			break;
	}
} // handleGAPEvent

#endif // CONFIG_BT_ENABLED
