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
#include "BLEUUID.h"
#include "BLEAdvertising.h"
#include "BLECharacteristic.h"
#include "BLEService.h"
#include "BLECharacteristicMap.h"
#include "BLEServiceMap.h"

struct profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t             gatts_if;
    uint16_t             app_id;
    uint16_t             conn_id;
    uint16_t             descr_handle;
    BLEUUID              *descr_uuid;
};

class BLEServer {
public:
	BLEServer(uint16_t appId, std::string deviceName);
	virtual ~BLEServer();

	void handleGAPEvent(
		esp_gap_ble_cb_event_t event,
		esp_ble_gap_cb_param_t *param);
	void handleGATTServerEvent(esp_gatts_cb_event_t event,
		esp_gatt_if_t gatts_if,
		esp_ble_gatts_cb_param_t *param);
	//void addCharacteristic(BLECharacteristic *characteristic, BLEService *service);
	void setDeviceName(std::string deviceName);
	void setUUID(uint8_t uuid[32]);
	void startAdvertising();

private:
	std::string         m_deviceName;
	uint16_t            m_appId;
	esp_ble_adv_data_t  m_adv_data;
	struct profile_inst m_profile;
	BLEAdvertising      m_bleAdvertising;
	//BLECharacteristicMap m_characteristicMap;
	BLEServiceMap        m_serviceMap;

};

#endif /* COMPONENTS_CPP_UTILS_BLESERVER_H_ */
