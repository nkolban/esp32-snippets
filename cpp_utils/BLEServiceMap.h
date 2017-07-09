/*
 * BLEServiceMap.h
 *
 *  Created on: Jun 22, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLESERVICEMAP_H_
#define COMPONENTS_CPP_UTILS_BLESERVICEMAP_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <map>
#include "BLEUUID.h"
#include <esp_gatts_api.h>

class BLEService;

class BLEServiceMap {
public:
	BLEServiceMap();
	virtual ~BLEServiceMap();
	void setByUUID(BLEUUID uuid, BLEService *service);
		void setByHandle(uint16_t handle, BLEService *service);
	BLEService *getByUUID(BLEUUID uuid);
	BLEService *getByHandle(uint16_t handle);
	std::string toString();
	void handleGATTServerEvent(
		esp_gatts_cb_event_t      event,
		esp_gatt_if_t             gatts_if,
		esp_ble_gatts_cb_param_t *param);
	private:
		std::map<std::string, BLEService *> m_uuidMap;
		std::map<uint16_t, BLEService *> m_handleMap;
};
#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLESERVICEMAP_H_ */
