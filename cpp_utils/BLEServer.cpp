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
#include <string.h>
#include <string>
#include <gatt_api.h>
#include <unordered_set>

static char tag[] = "BLEServer";

extern "C" {
	char *espToString(esp_err_t value);
}

//static esp_attr_value_t gatts_demo_char1_val;
//static uint8_t char1_str[] = {0x11,0x22,0x33};

/**
 * Construct a BLE Server
 */
BLEServer::BLEServer(uint16_t appId, std::string deviceName) {
	m_appId                                  = appId;
	m_deviceName                             = deviceName;
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
 * Start advertising.
 */
void BLEServer::startAdvertising() {
	m_bleAdvertising.setAppearance(3);
	m_bleAdvertising.start();
} // startAdvertising


/**
 * @brief Handle a GATT Server Event.
 * @param [in] event
 * @param [in] gatts_if
 * @param [in] param
 *
 */
void BLEServer::handleGATTServerEvent(
		esp_gatts_cb_event_t      event,
		esp_gatt_if_t             gatts_if,
		esp_ble_gatts_cb_param_t *param) {
// m_serviceMap.handleGATTServerEvent() ->
//   For each BLEService in the map -> handleGattServerEvent()
//      For each BLECharacteristic in the service -> handleGattServerEvent()
	ESP_LOGD(tag, "handleGATTServerEvent: %s",
			bt_utils_gatt_server_event_type_to_string(event).c_str());

	// Invoke the handler for every Service we have.
	m_serviceMap.handleGATTServerEvent(event, gatts_if, param);

	switch(event) {
		// ESP_GATTS_REG_EVT
		case ESP_GATTS_REG_EVT: {
			m_profile.gatts_if = gatts_if;
			ESP_LOGD(tag, "Registering device name: %s (esp_ble_gap_set_device_name)", m_deviceName.c_str());
			esp_err_t errRc = esp_ble_gap_set_device_name(m_deviceName.c_str());
			if (errRc != ESP_OK) {
				ESP_LOGE(tag, "esp_ble_gap_set_device_name: rc=%d %s", errRc, espToString(errRc));
				return;
			}

			startAdvertising();
			BLEUUID uuid((uint16_t)0x1234);
			BLEService *pService = new BLEService(uuid);
			m_serviceMap.setByUUID(uuid, pService);
			pService->create(gatts_if);
			break;
		} // ESP_GATTS_REG_EVT

		// ESP_GATTS_CREATE_EVT
		// create:
		// * esp_gatt_status_t status
		// * uint16_t service_handle
		// * esp_gatt_srvc_id_t service_id
		//
		case ESP_GATTS_CREATE_EVT: {
			BLEService *pService = m_serviceMap.getByUUID(param->create.service_id.id.uuid);
			m_serviceMap.setByHandle(param->create.service_handle, pService);
			pService->setHandle(param->create.service_handle);

			pService->start();

			BLECharacteristic *characteristic = new BLECharacteristic(BLEUUID((uint16_t)0x99AA));
			characteristic->setWritePermission(true);
			characteristic->setReadPermission(true);
			characteristic->setNotifyPermission(true);
			characteristic->setValue("hello steph");
			pService->addCharacteristic(characteristic);
			break;
		} // ESP_GATTS_CREATE_EVT


		// ESP_GATTS_READ_EVT - A request to read the value of a characteristic has arrived.
		//
		// read:
		// - uint16_t conn_id
		// - uint32_t trans_id
		// - esp_bd_addr_t bda
		// - uint16_t handle
		// - uint16_t offset
		// - bool is_long
		// - bool need_rsp
		//
		case ESP_GATTS_READ_EVT: {
			break;
		} // ESP_GATTS_READ_EVT


		// ESP_GATTS_WRITE_EVT - A request to write the value of a characteristic has arrived.
		//
		// write:
		// - uint16_t conn_id
		// - uint16_t trans_id
		// - esp_bd_addr_t bda
		// - uint16_t handle
		// - uint16_t offset
		// - bool need_rsp
		// - bool is_prep
		// - uint16_t len
		// - uint8_t *value

		case ESP_GATTS_WRITE_EVT: {
			break;
		}

		// ESP_GATTS_DISCONNECT_EVT
		case ESP_GATTS_DISCONNECT_EVT: {
			startAdvertising();
			break;
		} // ESP_GATTS_DISCONNECT_EVT


		// ESP_GATTS_ADD_CHAR_EVT - Indicate that a characteristic was added to the service.
		// add_char:
		// - esp_gatt_status_t status
		// - uint16_t attr_handle
		// - uint16_t service_handle
		// - esp_bt_uuid_t char_uuid
		case ESP_GATTS_ADD_CHAR_EVT: {
			break;
		}


		default:
			break;
	}
} // handleGATTServerEvent


/**
 * @brief Handle a receiver GAP event.
 * @param [in] event
 * @param [in] param
 */
void BLEServer::handleGAPEvent(
		esp_gap_ble_cb_event_t event,
		esp_ble_gap_cb_param_t *param) {
	ESP_LOGD(tag, "BLEServer ... handling GAP event!");
	switch(event) {
		case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT: {
			/*
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
			*/
			break;
		}
		default:
			break;
	}
} // handleGAPEvent

/*
void BLEServer::addCharacteristic(BLECharacteristic *characteristic, BLEService *pService) {
	ESP_LOGD(tag, "Adding characteristic (esp_ble_gatts_add_char): uuid=%s, serviceHandle=0x%.2x",
		characteristic->m_bleUUID.toString().c_str(),
		pService->getHandle());

	m_characteristicMap.setByUUID(characteristic->m_bleUUID, characteristic);

	esp_err_t errRc = ::esp_ble_gatts_add_char(
		pService->getHandle(),
		characteristic->getUUID().getNative(),
		(esp_gatt_perm_t)(ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE),
		characteristic->getProperties(),
		&characteristic->m_value,
		NULL);

	if (errRc != ESP_OK) {
		ESP_LOGE(tag, "esp_ble_gatts_add_char: rc=%d %s", errRc, espToString(errRc));
		return;
	}
}
*/

#endif // CONFIG_BT_ENABLED
