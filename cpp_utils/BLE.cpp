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

#include "BLEUtils.h"
#include "BLE.h"
#include "BLEDevice.h"
#include "BLEXXXCharacteristic.h"

static char LOG_TAG[] = "BLE";

extern "C" {
	char *espToString(esp_err_t value);
}

static EventGroupHandle_t g_eventGroup;
#define EVENT_GROUP_SCAN_COMPLETE (1<<0)


//static esp_gatt_if_t g_gattc_if;

/*
 * We maintain a map of found devices.  The map is keyed off the 6 byte address value of the device.
 * Note that this address is binary and should not be directly printed.  The value of the map is
 * the BLEDevice object that this device represents.
 */
static std::map<ble_address, BLEDevice> g_devices;

BLEServer *BLE::m_bleServer;

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
 * https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile
 */
/*
static const char *adv_type_to_string(uint8_t advType) {
	switch(advType) {
	case ESP_BLE_AD_TYPE_FLAG:
		return "ESP_BLE_AD_TYPE_FLAG";
	case ESP_BLE_AD_TYPE_16SRV_PART:
		return "ESP_BLE_AD_TYPE_16SRV_PART";
	case ESP_BLE_AD_TYPE_16SRV_CMPL:
		return "ESP_BLE_AD_TYPE_16SRV_CMPL";
	case ESP_BLE_AD_TYPE_32SRV_PART:
		return "ESP_BLE_AD_TYPE_32SRV_PART";
	case ESP_BLE_AD_TYPE_32SRV_CMPL:
		return "ESP_BLE_AD_TYPE_32SRV_CMPL";
	case ESP_BLE_AD_TYPE_128SRV_PART:
		return "ESP_BLE_AD_TYPE_128SRV_PART";
	case ESP_BLE_AD_TYPE_128SRV_CMPL:
		return "ESP_BLE_AD_TYPE_128SRV_CMPL";
	case ESP_BLE_AD_TYPE_NAME_SHORT:
		return "ESP_BLE_AD_TYPE_NAME_SHORT";
	case ESP_BLE_AD_TYPE_NAME_CMPL:
		return "ESP_BLE_AD_TYPE_NAME_CMPL";
	case ESP_BLE_AD_TYPE_TX_PWR:
		return "ESP_BLE_AD_TYPE_TX_PWR";
	case ESP_BLE_AD_TYPE_DEV_CLASS:
		return "ESP_BLE_AD_TYPE_DEV_CLASS";
	case ESP_BLE_AD_TYPE_SM_TK:
		return "ESP_BLE_AD_TYPE_SM_TK";
	case ESP_BLE_AD_TYPE_SM_OOB_FLAG:
		return "ESP_BLE_AD_TYPE_SM_OOB_FLAG";
	case ESP_BLE_AD_TYPE_INT_RANGE:
		return "ESP_BLE_AD_TYPE_INT_RANGE";
	case ESP_BLE_AD_TYPE_SOL_SRV_UUID:
		return "ESP_BLE_AD_TYPE_SOL_SRV_UUID";
	case ESP_BLE_AD_TYPE_128SOL_SRV_UUID:
		return "ESP_BLE_AD_TYPE_128SOL_SRV_UUID";
	case ESP_BLE_AD_TYPE_SERVICE_DATA:
		return "ESP_BLE_AD_TYPE_SERVICE_DATA";
	case ESP_BLE_AD_TYPE_PUBLIC_TARGET:
		return "ESP_BLE_AD_TYPE_PUBLIC_TARGET";
	case ESP_BLE_AD_TYPE_RANDOM_TARGET:
		return "ESP_BLE_Amap1D_TYPE_RANDOM_TARGET";
	case ESP_BLE_AD_TYPE_APPEARANCE:
		return "ESP_BLE_AD_TYPE_APPEARANCE";
	case ESP_BLE_AD_TYPE_ADV_INT:
		return "ESP_BLE_AD_TYPE_ADV_INT";
	case ESP_BLE_AD_TYPE_32SOL_SRV_UUID:
		return "ESP_BLE_AD_TYPE_32SOL_SRV_UUID";
	case ESP_BLE_AD_TYPE_32SERVICE_DATA:
		return "ESP_BLE_AD_TYPE_32SERVICE_DATA";
	case ESP_BLE_AD_TYPE_128SERVICE_DATA:
		return "ESP_BLE_AD_TYPE_128SERVICE_DATA";
	case ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE:
		return "ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE";
	default:
		ESP_LOGD(tag, "Unknown adv data type: 0x%x", advType);
		return "Unknown";
	}
}
*/

/**
 * @brief Dump the advertizing payload.
 *
 * The payload is a buffer of bytes that is either 31 bytes long or terminated by
 * a 0 length value.  Each entry in the buffer has the format:
 * [length][type][data...]
 *
 * The length does not include itself but does include everything after it until the next record.  A record
 * with a length value of 0 indicates a terminator.
 */
/*
static void dump_adv_payload(uint8_t *payload) {
	uint8_t length;
	uint8_t ad_type;
	uint8_t sizeConsumed = 0;
	bool finished = false;
	//int i;
	//char text[31*2+1];
	//sprintf(text, "%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x")
	while(!finished) {
		length = *payload;
		payload++;
		if (length != 0) {
			ad_type = *payload;
			payload += length;
			ESP_LOGD(tag, "Type: 0x%.2x (%s), length: %d", ad_type, adv_type_to_string(ad_type), length);

		}

		sizeConsumed = 1 + length;
		if (sizeConsumed >=31 || length == 0) {
			finished = true;
		}
	} // !finished
} // dump_adv_payload
*/


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
		BLEDevice *pDevice = BLEUtils::findByAddress(std::string((char *)param->open.remote_bda, 6));
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
		BLEDevice *pDevice = BLEUtils::findByConnId(param->search_res.conn_id);
		pDevice->addService(param->search_res.srvc_id);
		break;
	}

	case ESP_GATTC_SEARCH_CMPL_EVT: {
		BLEDevice *pDevice = BLEUtils::findByConnId(param->search_cmpl.conn_id);
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
			BLEDevice *pDevice = BLEUtils::findByConnId(param->get_char.conn_id);
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
			BLEDevice *pDevice = BLEUtils::findByConnId(param->read.conn_id);
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
		case ESP_GAP_BLE_SCAN_RESULT_EVT: {
			BLEDevice device;

			ESP_LOGD(LOG_TAG, "search_evt: %s", bt_gap_search_event_type_to_string(param->scan_rst.search_evt).c_str());

			if (param->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_CMPL_EVT) {
				//ESP_LOGD(tag, "num_resps: %d", param->scan_rst.num_resps);
				//BLE::dumpDevices();
				xEventGroupSetBits(g_eventGroup, EVENT_GROUP_SCAN_COMPLETE);
			}
			else if (param->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_RES_EVT) {
				//ESP_LOGD(tag, "device type: %s", bt_dev_type_to_string(param->scan_rst.dev_type));
				//ESP_LOGD(tag, "device address (bda): %02x:%02x:%02x:%02x:%02x:%02x", BT_BD_ADDR_HEX(param->scan_rst.bda));
				//ESP_LOGD(tag, "rssi: %d", param->scan_rst.rssi);
				//ESP_LOGD(tag, "addr_type: %s", bt_addr_t_to_string(param->scan_rst.ble_addr_type));
				//ESP_LOGD(tag, "flag: %d", param->scan_rst.flag);
				device.setAddress(std::string((char *)param->scan_rst.bda, 6));
				device.setRSSI(param->scan_rst.rssi);
				device.setAdFlag(param->scan_rst.flag);
				//device.dump();
				device.parsePayload((uint8_t *)param->scan_rst.ble_adv);
				g_devices.insert(std::pair<std::string,BLEDevice>(device.getAddress(),device));
				//dump_adv_payload(param->scan_rst.ble_adv);
			} else {
				ESP_LOGD(LOG_TAG, "Unhandled search_evt type!");
			}
			break;
		} // ESP_GAP_BLE_SCAN_RESULT_EVT

		case ESP_GAP_BLE_SEC_REQ_EVT: {
			esp_err_t errRc = ::esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
			if (errRc != ESP_OK) {
				ESP_LOGE(LOG_TAG, "esp_ble_gap_security_rsp: rc=%d %s", errRc, espToString(errRc));
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
} // gap_event_handler


/**
 * @brief Get the current set of known devices.
 */
std::map<ble_address, BLEDevice> getDevices() {
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
		ESP_LOGE(LOG_TAG, "esp_bt_controller_init: rc=%d %s", errRc, espToString(errRc));
		return;
	}


	errRc = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_bt_controller_enable: rc=%d %s", errRc, espToString(errRc));
		return;
	}

	errRc = esp_bluedroid_init();
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_bluedroid_init: rc=%d %s", errRc, espToString(errRc));
		return;
	}

	errRc = esp_bluedroid_enable();
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_bluedroid_enable: rc=%d %s", errRc, espToString(errRc));
		return;
	}

	errRc = esp_ble_gap_register_callback(gap_event_handler);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gap_register_callback: rc=%d %s", errRc, espToString(errRc));
		return;
	}

	errRc = esp_ble_gatts_register_callback(gatt_server_event_handler);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gatts_register_callback: rc=%d %s", errRc, espToString(errRc));
		return;
	}

	errRc = ::esp_ble_gap_set_device_name(deviceName.c_str());
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gap_set_device_name: rc=%d %s", errRc, espToString(errRc));
		return;
	};

	esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;
	errRc = ::esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gap_set_security_param: rc=%d %s", errRc, espToString(errRc));
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
		ESP_LOGE(LOG_TAG, "esp_bt_controller_init: rc=%d %s", errRc, espToString(errRc));
		return;
	}


	errRc = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_bt_controller_enable: rc=%d %s", errRc, espToString(errRc));
		return;
	}

	errRc = esp_bluedroid_init();
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_bluedroid_init: rc=%d %s", errRc, espToString(errRc));
		return;
	}

	errRc = esp_bluedroid_enable();
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_bluedroid_enable: rc=%d %s", errRc, espToString(errRc));
		return;
	}

	errRc = esp_ble_gap_register_callback(gap_event_handler);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gap_register_callback: rc=%d %s", errRc, espToString(errRc));
		return;
	}

	errRc = esp_ble_gattc_register_callback(gatt_client_event_handler);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gattc_register_callback: rc=%d %s", errRc, espToString(errRc));
		return;
	}

	errRc = esp_ble_gattc_app_register(0);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gattc_app_register: rc=%d %s", errRc, espToString(errRc));
		return;
	}
	g_eventGroup = xEventGroupCreate();
	xEventGroupClearBits(g_eventGroup, 0xff);
} // init



/**
 * @brief Perform a %BLE scan.
 *
 * We scan for BLE devices that are advertizing.
 *
 * @param [in] duration The duration that the scan is to run for measured in seconds.
 * @param [in] scanType The type of scanning requested.  The choices are `BLE_SCA_TYPE_PASSIVE` and `BLE_SCAN_TYPE_ACTIVE`.
 * The distinction between them is whether or not the advertizer has a scan response requested from it.
 */
void BLE::scan(int duration, esp_ble_scan_type_t scan_type) {
	g_devices.clear();
	static esp_ble_scan_params_t ble_scan_params;
	ble_scan_params.scan_type              = scan_type;
	ble_scan_params.own_addr_type          = BLE_ADDR_TYPE_PUBLIC;
	ble_scan_params.scan_filter_policy     = BLE_SCAN_FILTER_ALLOW_ALL;
	ble_scan_params.scan_interval          = 0x50;
	ble_scan_params.scan_window            = 0x30;
	esp_err_t errRc = esp_ble_gap_set_scan_params(&ble_scan_params);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gap_set_scan_params: rc=%d %s", errRc, espToString(errRc));
		return;
	}

	errRc = esp_ble_gap_start_scanning(duration);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gap_start_scanning: rc=%d", errRc);
		return;
	}
	/*
	 * Block until done
	 */
	xEventGroupWaitBits(g_eventGroup,
		EVENT_GROUP_SCAN_COMPLETE,
		1, // Clear on exit
		0, // Wait for all bits
		portMAX_DELAY);
	ESP_LOGD(LOG_TAG, "Scan complete! - BLE:scan() returning.");
} // scan

#endif // CONFIG_BT_ENABLED
