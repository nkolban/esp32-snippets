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

class BLEServerCallbacks;


/**
 * @brief A data structure that manages the %BLE servers owned by a BLE server.
 */
class BLEServiceMap {
public:
	void setByUUID(BLEUUID uuid, BLEService* service);
	void setByHandle(uint16_t handle, BLEService* service);
	BLEService* getByUUID(BLEUUID uuid);
	BLEService* getByHandle(uint16_t handle);
	std::string toString();
	void handleGATTServerEvent(
		esp_gatts_cb_event_t      event,
		esp_gatt_if_t             gatts_if,
		esp_ble_gatts_cb_param_t *param);
	private:
		std::map<std::string, BLEService*> m_uuidMap;
		std::map<uint16_t, BLEService*>    m_handleMap;
};


/**
 * @brief The model of a %BLE server.
 */
class BLEServer {
public:
	BLEServer();


	BLEService*     createService(BLEUUID uuid);
	BLEAdvertising* getAdvertising();
	void            setCallbacks(BLEServerCallbacks *pCallbacks);
	void            startAdvertising();


private:
	friend class BLEService;
	friend class BLECharacteristic;
	friend class BLE;
	esp_ble_adv_data_t  m_adv_data;
	uint16_t            m_appId;
	BLEAdvertising      m_bleAdvertising;
  uint16_t            m_gatts_if;
  uint16_t						m_connId;
	FreeRTOS::Semaphore m_semaphoreRegisterAppEvt = FreeRTOS::Semaphore("RegisterAppEvt");
	FreeRTOS::Semaphore m_semaphoreCreateEvt = FreeRTOS::Semaphore("CreateEvt");
	BLEServiceMap       m_serviceMap;
	BLEServerCallbacks* m_pServerCallbacks;

	void            createApp(uint16_t appId);
	uint16_t        getConnId();
	uint16_t        getGattsIf();
	void            handleGAPEvent(esp_gap_ble_cb_event_t event,	esp_ble_gap_cb_param_t *param);
	void            handleGATTServerEvent(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
	void            registerApp();
}; // BLEServer


/**
 * @brief Callbacks associated with the operation of a %BLE server.
 */
class BLEServerCallbacks {
public:
	virtual ~BLEServerCallbacks() {};
	/**
	 * @brief Handle a new client connection.
	 *
	 * When a new client connects, we are invoked.
	 *
	 * @param [in] pServer A reference to the %BLE server that received the client connection.
	 */
	virtual void onConnect(BLEServer* pServer);

	/**
	 * @brief Handle an existing client disconnection.
	 *
	 * When an existing client disconnects, we are invoked.
	 *
	 * @param [in] pServer A reference to the %BLE server that received the existing client disconnection.
	 */
	virtual void onDisconnect(BLEServer* pServer);
}; // BLEServerCallbacks



#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLESERVER_H_ */
