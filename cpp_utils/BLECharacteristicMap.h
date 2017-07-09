/*
 * BLECharacteristicMap.h
 *
 *  Created on: Jun 22, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLECHARACTERISTICMAP_H_
#define COMPONENTS_CPP_UTILS_BLECHARACTERISTICMAP_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <map>
#include "BLECharacteristic.h"

class BLECharacteristicMap {
public:
	BLECharacteristicMap();
	virtual ~BLECharacteristicMap();
	void setByUUID(BLEUUID uuid, BLECharacteristic *characteristic);
	void setByHandle(uint16_t handle, BLECharacteristic *characteristic);
	BLECharacteristic *getByUUID(BLEUUID uuid);
	BLECharacteristic *getByHandle(uint16_t handle);
	BLECharacteristic *getFirst();
	BLECharacteristic *getNext();
	std::string toString();
	void handleGATTServerEvent(
			esp_gatts_cb_event_t      event,
			esp_gatt_if_t             gatts_if,
			esp_ble_gatts_cb_param_t *param);


private:
	std::map<std::string, BLECharacteristic *> m_uuidMap;
	std::map<uint16_t, BLECharacteristic *> m_handleMap;
	std::map<std::string, BLECharacteristic *>::iterator m_iterator;
};

#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLECHARACTERISTICMAP_H_ */
