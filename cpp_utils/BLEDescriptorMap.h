/*
 * BLEDescriptorMap.h
 *
 *  Created on: Jun 22, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLEDESCRIPTORMAP_H_
#define COMPONENTS_CPP_UTILS_BLEDESCRIPTORMAP_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <map>
#include "BLEUUID.h"
#include <esp_gatts_api.h>   // ESP32 BLE

class BLEDescriptor;

class BLEDescriptorMap {
public:
	BLEDescriptorMap();
	virtual ~BLEDescriptorMap();
	void setByUUID(BLEUUID uuid,      BLEDescriptor *pDescriptor);
	void setByHandle(uint16_t handle, BLEDescriptor *pDescriptor);
	BLEDescriptor *getByUUID(BLEUUID uuid);
	BLEDescriptor *getByHandle(uint16_t handle);
	std::string toString();
	void handleGATTServerEvent(
			esp_gatts_cb_event_t      event,
			esp_gatt_if_t             gatts_if,
			esp_ble_gatts_cb_param_t *param);
	BLEDescriptor *getFirst();
	BLEDescriptor *getNext();
private:
	std::map<std::string, BLEDescriptor *> m_uuidMap;
	std::map<uint16_t,    BLEDescriptor *> m_handleMap;
	std::map<std::string, BLEDescriptor *>::iterator m_iterator;
};
#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLEDESCRIPTORMAP_H_ */
