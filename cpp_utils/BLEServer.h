/*
 * BLEServer.h
 *
 *  Created on: Apr 16, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLESERVER_H_
#define COMPONENTS_CPP_UTILS_BLESERVER_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <string>
#include <string.h>
#include <esp_gatts_api.h>
#include "FreeRTOS.h"
#include "BLEUUID.h"
#include "BLEAdvertising.h"
#include "BLECharacteristic.h"
#include "BLEService.h"
#include "BLECharacteristicMap.h"
#include "BLEServiceMap.h"
#include "BLEServerCallbacks.h"

class BLEServerCallbacks;
class BLEServer {
public:
	BLEServer();
	virtual ~BLEServer();

	void            createApp(uint16_t appId);
	BLEService     *createService(BLEUUID uuid);
	BLEAdvertising *getAdvertising();
	uint16_t        getConnId();
	uint16_t        getGattsIf();
	void            handleGAPEvent(esp_gap_ble_cb_event_t event,	esp_ble_gap_cb_param_t *param);
	void            handleGATTServerEvent(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
	void            registerApp();
	void            setCallbacks(BLEServerCallbacks *pCallbacks);
	void            setUUID(uint8_t uuid[32]);
	void            startAdvertising();


private:
	esp_ble_adv_data_t  m_adv_data;
	uint16_t            m_appId;
	BLEAdvertising      m_bleAdvertising;
  uint16_t            m_gatts_if;
  uint16_t						m_connId;
	FreeRTOS::Semaphore m_serializeMutex;
	BLEServiceMap       m_serviceMap;
	BLEServerCallbacks *m_pServerCallbacks;
}; // BLEServer
#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLESERVER_H_ */
