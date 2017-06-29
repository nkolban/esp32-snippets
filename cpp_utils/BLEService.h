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
#include "FreeRTOS.h"
#include "BLEUUID.h"
#include "BLECharacteristic.h"
#include "BLECharacteristicMap.h"


class BLEService {
public:
	BLEService(BLEUUID uuid);
	virtual ~BLEService();

	void addCharacteristic(BLECharacteristic *pCharacteristic);
	BLECharacteristic *getCharacteristic(BLEUUID uuid);
	void executeCreate(esp_gatt_if_t gatts_if);
	void dump();
	esp_gatt_srvc_id_t getService();
	BLEUUID getUUID();
	void setService(esp_gatt_srvc_id_t srvc_id);
	void start();
	std::string toString();

private:
	esp_gatt_srvc_id_t   m_srvc_id;
	BLEUUID              m_uuid;
	esp_gatt_if_t        m_gatts_if;
	uint16_t             m_handle;
	BLECharacteristicMap m_characteristicMap;
	BLECharacteristic   *m_lastCreatedCharacteristic;
	friend class BLEServer;
	friend class BLEServiceMap;
	friend class BLEDescriptor;
	friend class BLECharacteristic;

	BLECharacteristic *getLastCreatedCharacteristic();
	void setHandle(uint16_t handle);
	uint16_t getHandle();
	void handleGATTServerEvent(
			esp_gatts_cb_event_t      event,
			esp_gatt_if_t             gatts_if,
			esp_ble_gatts_cb_param_t *param);
	FreeRTOS::Semaphore m_serializeMutex;

};
#endif // CONFIG_BT_ENABLED
#endif /* COMPONENTS_CPP_UTILS_BLESERVICE_H_ */
