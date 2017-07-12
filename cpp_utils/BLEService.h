/*
 * BLEService.h
 *
 *  Created on: Mar 25, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLESERVICE_H_
#define COMPONENTS_CPP_UTILS_BLESERVICE_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include <esp_gatts_api.h>

#include "BLECharacteristic.h"
#include "BLECharacteristicMap.h"
#include "BLEServer.h"
#include "BLEUUID.h"
#include "FreeRTOS.h"

class BLEServer;

class BLEService {
public:
	BLEService(BLEUUID uuid);
	virtual ~BLEService();

	void               addCharacteristic(BLECharacteristic *pCharacteristic);
	BLECharacteristic* createCharacteristic(BLEUUID uuid, uint32_t properties);
	void               dump();
	void               executeCreate(BLEServer *pServer);
	BLECharacteristic* getCharacteristic(BLEUUID uuid);
	BLEUUID            getUUID();
	BLEServer*         getServer();
	void               start();
	std::string        toString();

private:
	friend class BLEServer;
	friend class BLEServiceMap;
	friend class BLEDescriptor;
	friend class BLECharacteristic;
	friend class BLEDevice;

	BLECharacteristicMap m_characteristicMap;
	uint16_t             m_handle;
	BLECharacteristic*   m_lastCreatedCharacteristic;
	BLEServer*           m_pServer;
	FreeRTOS::Semaphore  m_serializeMutex;
	BLEUUID              m_uuid;

	uint16_t           getHandle();
	BLECharacteristic *getLastCreatedCharacteristic();
	void               handleGATTServerEvent(
		esp_gatts_cb_event_t      event,
		esp_gatt_if_t             gatts_if,
		esp_ble_gatts_cb_param_t *param);
	void               setHandle(uint16_t handle);
	//void               setService(esp_gatt_srvc_id_t srvc_id);
}; // BLEService

#endif // CONFIG_BT_ENABLED
#endif /* COMPONENTS_CPP_UTILS_BLESERVICE_H_ */
