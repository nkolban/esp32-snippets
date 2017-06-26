/*
 * BLEUtils.h
 *
 *  Created on: Mar 25, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLEUTILS_H_
#define COMPONENTS_CPP_UTILS_BLEUTILS_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <esp_gattc_api.h>   // ESP32 BLE
#include <esp_gatts_api.h>   // ESP32 BLE
#include <string>
#include <BLEDevice.h>
typedef std::string ble_address;

class BLEUtils {
public:
	BLEUtils();
	virtual ~BLEUtils();
	static std::string addressToString(ble_address address);
	static std::string addressToString(esp_bd_addr_t address);
	static ble_address parseAddress(std::string uuid);
	static esp_gatt_id_t buildGattId(esp_bt_uuid_t uuid, uint8_t inst_id=0);
	static esp_gatt_srvc_id_t buildGattSrvcId(esp_gatt_id_t gattId, bool is_primary=true);
	static esp_bt_uuid_t buildUUID(std::string uuid);
	static esp_bt_uuid_t buildUUID(uint16_t uuid);
	static esp_bt_uuid_t buildUUID(uint32_t uuid);
	static void dumpHexData(uint8_t *target, uint8_t *source, uint8_t length);
	static BLEDevice *findByConnId(uint16_t conn_id);
	static BLEDevice *findByAddress(ble_address address);
	static std::string gattServiceIdToString(esp_gatt_srvc_id_t srvcId);
	static std::string gattStatusToString(esp_gatt_status_t status);
	static std::string gattServiceToString(uint32_t serviceId);
	static void registerByAddress(ble_address address, BLEDevice *pDevice);
	static void registerByConnId(uint16_t conn_id, BLEDevice *pDevice);
	static std::string uuidToString(esp_bt_uuid_t uuid);
	static std::string gattCharacteristicUUIDToString(uint32_t characteristicUUID);
	static void dumpGattClientEvent(
		esp_gattc_cb_event_t event,
		esp_gatt_if_t gattc_if,
		esp_ble_gattc_cb_param_t *evtParam);
	static void dumpGattServerEvent(
		esp_gatts_cb_event_t event,
		esp_gatt_if_t gatts_if,
		esp_ble_gatts_cb_param_t *evtParam);
	static std::string devTypeToString(esp_bt_dev_type_t type);
};

std::string bt_event_type_to_string(uint32_t eventType);
std::string bt_utils_gatt_client_event_type_to_string(esp_gattc_cb_event_t eventType);
std::string bt_utils_gatt_server_event_type_to_string(esp_gatts_cb_event_t eventType);
std::string bt_gap_search_event_type_to_string(uint32_t searchEvt);
#endif // CONFIG_BT_ENABLED
#endif /* COMPONENTS_CPP_UTILS_BLEUTILS_H_ */
