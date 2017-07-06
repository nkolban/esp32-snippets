/*
 * BLE.cpp
 *
 *  Created on: Mar 16, 2017
 *      Author: kolban
 */
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <bt.h>              // ESP32 BLE
#include <esp_bt_main.h>     // ESP32 BLE
#include <esp_gap_ble_api.h> // ESP32 BLE
#include <esp_gattc_api.h>   // ESP32 BLE
#include <esp_gatts_api.h>   // ESP32 BLE
#include <esp_err.h>         // ESP32 ESP-IDF
#include <esp_log.h>         // ESP32 ESP-IDF
#include <map>               // Part of C++ STL
#include <sstream>
#include <iomanip>

#include "BLE.h"
#include "BLERemoteDevice.h"
#include "BLEUtils.h"
#include "BLEXXXCharacteristic.h"
#include "GeneralUtils.h"

static char LOG_TAG[] = "BLE";


static EventGroupHandle_t g_eventGroup;
#define EVENT_GROUP_SCAN_COMPLETE (1<<0)


//static esp_gatt_if_t g_gattc_if;

/*
 * We maintain a map of found devices.  The map is keyed off the 6 byte address value of the device.
 * Note that this address is binary and should not be directly printed.  The value of the map is
 * the BLEDevice object that this device represents.
 */
static std::map<std::string, BLERemoteDevice> g_devices;

BLEServer *BLE::m_bleServer = nullptr;
BLEScan   *BLE::m_pScan     = nullptr;

BLE::BLE() {
}


BLE::~BLE() {
}


/**
 * @brief Dump all the devices.
 *
 * During scanning, a map is built containing all the unique devices found.  Calling this function
 * will dump the details of all the devices.
 */
void BLE::dumpDevices() {
	ESP_LOGD(LOG_TAG, "Number of devices: %d", g_devices.size());
    for (auto &myPair : g_devices ) {
    	myPair.second.dump();
    }
} // dumpDevices


/**
 * @brief Handle GATT server events.
 *
 * @param [in] event
 * @param [in] gatts_if
 * @param [in] param
 */
void gatt_server_event_handler(
   esp_gatts_cb_event_t      event,
   esp_gatt_if_t             gatts_if,
   esp_ble_gatts_cb_param_t *param
) {
	ESP_LOGD(LOG_TAG, "gatt_server_event_handler [esp_gatt_if: %d] ... %s",
		gatts_if,
		bt_utils_gatt_server_event_type_to_string(event).c_str());
	BLEUtils::dumpGattServerEvent(event, gatts_if, param);
	if (BLE::m_bleServer != nullptr) {
		BLE::m_bleServer->handleGATTServerEvent(event, gatts_if, param);
	}
} // gatt_server_event_handler


/**
 * @brief Handle GATT client events.
 *
 * Handler for the GATT client events.
 * * `ESP_GATTC_OPEN_EVT` – Invoked when a connection is opened.
 * * `ESP_GATTC_PREP_WRITE_EVT` – Response to write a characteristic.
 * * `ESP_GATTC_READ_CHAR_EVT` – Response to read a characteristic.
 * * `ESP_GATTC_REG_EVT` – Invoked when a GATT client has been registered.
 *
 * @param [in] event
 * @param [in] gattc_if
 * @param [in] param
 */
static void gatt_client_event_handler(
	esp_gattc_cb_event_t event,
	esp_gatt_if_t gattc_if,
	esp_ble_gattc_cb_param_t *param) {

	ESP_LOGD(LOG_TAG, "gatt_client_event_handler [esp_gatt_if: %d] ... %s",
		gattc_if, bt_utils_gatt_client_event_type_to_string(event).c_str());
	BLEUtils::dumpGattClientEvent(event, gattc_if, param);

	switch(event) {
	case ESP_GATTC_OPEN_EVT: {
		BLERemoteDevice *pDevice = BLEUtils::findByAddress(std::string((char *)param->open.remote_bda, 6));
		BLEUtils::registerByConnId(param->open.conn_id, pDevice);
		pDevice->dump();
		pDevice->onConnected(param->open.status);
		break;
	}

	/*
	 * The search_res field of the parameter has been populated.  It contains:
	 * * uint16_t conn_id - Can this be used to find the device?
	 * * esp_gatt_srvc_id_t srvc_id
	 *   * esp_gatt_id_t id
	 *      * esp_bt_uuid_t uuid
	 *      * uint8_t inst_id
	 *   * bool is_primary
	 */
	case ESP_GATTC_SEARCH_RES_EVT: {
		BLERemoteDevice *pDevice = BLEUtils::findByConnId(param->search_res.conn_id);
		pDevice->addService(param->search_res.srvc_id);
		break;
	}

	case ESP_GATTC_SEARCH_CMPL_EVT: {
		BLERemoteDevice *pDevice = BLEUtils::findByConnId(param->search_cmpl.conn_id);
		pDevice->onSearchComplete();
		break;
	}

	/*
	 * The `get_char` field of the parameters has been populated.  It contains:
	 * * esp_gatt_status_t status
	 * * uint16_t conn_id
	 * * esp_gatt_srvc_id_t srvc_id
	 * * esp_gatt_id_t char_id
	 * * esp_gatt_char_prop_t char_prop
	 */
	case ESP_GATTC_GET_CHAR_EVT: {
		if (param->get_char.status == ESP_GATT_OK) {
			BLERemoteDevice *pDevice = BLEUtils::findByConnId(param->get_char.conn_id);
			BLECharacteristicXXX characteristic(param->get_char.conn_id,
					param->get_char.srvc_id, param->get_char.char_id, param->get_char.char_prop);
			pDevice->onCharacteristic(characteristic);
		}
		break;
	}


	/*
	 * The `read` field of the parameters has been populated.  It contains:
	 * * esp_gatt_status_t status
	 * * uint16_t conn_id
	 * * esp_gatt_srvc_id_t srvc_id
	 * * esp_gatt_id_t char_id
	 * * esp_gatt_id_t descr_id
	 * * uint8_t *value
	 * * uint16_t value_type
	 * * uint16_t value_len
	 */
	case ESP_GATTC_READ_CHAR_EVT: {
		if (param->read.status == ESP_GATT_OK) {
			BLERemoteDevice *pDevice = BLEUtils::findByConnId(param->read.conn_id);
			std::string data = std::string((char *)param->read.value, param->read.value_len);
			pDevice->onRead(data);
		}
		break;
	}

	default:
		break;
	}

} // gatt_event_handler


/**
 * @brief Handle GAP events.
 */
static void gap_event_handler(
	esp_gap_ble_cb_event_t event,
	esp_ble_gap_cb_param_t *param) {

	BLEUtils::dumpGapEvent(event, param);

	switch(event) {
		case ESP_GAP_BLE_SEC_REQ_EVT: {
			esp_err_t errRc = ::esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
			if (errRc != ESP_OK) {
				ESP_LOGE(LOG_TAG, "esp_ble_gap_security_rsp: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
			}
			break;
		}

		default: {
			break;
		}
	} // switch

	if (BLE::m_bleServer != nullptr) {
		BLE::m_bleServer->handleGAPEvent(event, param);
	}

	if (BLE::getScan() != nullptr) {
		BLE::getScan()->gapEventHandler(event, param);
	}
} // gap_event_handler


/**
 * @brief Get the current set of known devices.
 */
std::map<std::string, BLERemoteDevice> getDevices() {
	return g_devices;
} // getDevices


/**
 * @brief Initialize the server %BLE environment.
 *
 */
 void BLE::initServer(std::string deviceName) {
	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	esp_err_t errRc = esp_bt_controller_init(&bt_cfg);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_bt_controller_init: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}


	errRc = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_bt_controller_enable: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}

	errRc = esp_bluedroid_init();
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_bluedroid_init: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}

	errRc = esp_bluedroid_enable();
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_bluedroid_enable: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}

	errRc = esp_ble_gap_register_callback(gap_event_handler);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gap_register_callback: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}

	errRc = esp_ble_gatts_register_callback(gatt_server_event_handler);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gatts_register_callback: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}

	errRc = ::esp_ble_gap_set_device_name(deviceName.c_str());
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gap_set_device_name: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	};

	esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;
	errRc = ::esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gap_set_security_param: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	};

	return;
}


/**
 * @brief Initialize the client %BLE environment.
 */
void BLE::initClient() {
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  esp_err_t errRc = esp_bt_controller_init(&bt_cfg);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_bt_controller_init: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}


	errRc = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_bt_controller_enable: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}

	errRc = esp_bluedroid_init();
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_bluedroid_init: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}

	errRc = esp_bluedroid_enable();
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_bluedroid_enable: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}

	errRc = esp_ble_gap_register_callback(gap_event_handler);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gap_register_callback: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}

	errRc = esp_ble_gattc_register_callback(gatt_client_event_handler);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gattc_register_callback: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}

	errRc = esp_ble_gattc_app_register(0);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gattc_app_register: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}
	g_eventGroup = xEventGroupCreate();
	xEventGroupClearBits(g_eventGroup, 0xff);
} // init

BLEScan* BLE::getScan() {
	if (m_pScan == nullptr) {
		m_pScan = new BLEScan();
	}
	return m_pScan;
} // getScan

#endif // CONFIG_BT_ENABLED
