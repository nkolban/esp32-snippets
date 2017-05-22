/*
 * BLEServer.h
 *
 *  Created on: Apr 16, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLESERVER_H_
#define COMPONENTS_CPP_UTILS_BLESERVER_H_
#include <string>
#include <string.h>
#include <esp_gatts_api.h>

struct profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

class BLEServer {
public:
	BLEServer(uint16_t appId, std::string deviceName);
	virtual ~BLEServer();
	void setDeviceName(std::string deviceName) {
		m_deviceName = deviceName;
	}
	void handleGATTServerEvent(esp_gatts_cb_event_t event,
		esp_gatt_if_t gatts_if,
		esp_ble_gatts_cb_param_t *param);
	void handleGAPEvent(
		esp_gap_ble_cb_event_t event,
		esp_ble_gap_cb_param_t *param);
	void setUUID(uint8_t uuid[32]) {
		memcpy(m_uuid, uuid, 32);
	}
private:
	std::string m_deviceName;
	uint16_t m_appId;
	uint8_t m_uuid[32];
	esp_ble_adv_data_t m_adv_data;
	struct profile_inst m_profile;
};

#endif /* COMPONENTS_CPP_UTILS_BLESERVER_H_ */
