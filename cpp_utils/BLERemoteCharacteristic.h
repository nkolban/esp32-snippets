/*
 * BLERemoteCharacteristic.h
 *
 *  Created on: Jul 8, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLEREMOTECHARACTERISTIC_H_
#define COMPONENTS_CPP_UTILS_BLEREMOTECHARACTERISTIC_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include <string>

#include <esp_gattc_api.h>

#include "BLERemoteService.h"
#include "BLEUUID.h"
#include "FreeRTOS.h"

class BLERemoteService;

/**
 * @brief A model of a remote %BLE characteristic.
 */
class BLERemoteCharacteristic {
public:
	BLERemoteCharacteristic(esp_gatt_id_t charId, esp_gatt_char_prop_t charProp, BLERemoteService* pRemoteService);

	// Public member functions
	BLEUUID     getUUID();
	std::string readValue(void);
	uint8_t     readUInt8(void);
	uint16_t    readUInt16(void);
	uint32_t    readUInt32(void);
	void        registerForNotify(void (*notifyCallback)(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify));
	void        writeValue(uint8_t* data, size_t length, bool response = false);
	void        writeValue(std::string newValue, bool response = false);
	void        writeValue(uint8_t newValue, bool response = false);
	std::string toString(void);

private:
	friend class BLEClient;
	friend class BLERemoteService;

	// Private member functions
	void gattClientEventHandler(
		esp_gattc_cb_event_t      event,
		esp_gatt_if_t             gattc_if,
		esp_ble_gattc_cb_param_t *evtParam);

	// Private properties
	esp_gatt_id_t        m_charId;
	esp_gatt_char_prop_t m_charProp;
	BLERemoteService*    m_pRemoteService;
	FreeRTOS::Semaphore  m_semaphoreReadCharEvt      = FreeRTOS::Semaphore("ReadCharEvt");
	FreeRTOS::Semaphore  m_semaphoreRegForNotifyEvt  = FreeRTOS::Semaphore("RegForNotifyEvt");
	FreeRTOS::Semaphore  m_semaphoreWriteCharEvt     = FreeRTOS::Semaphore("WriteCharEvt");
	std::string          m_value;
  void (*m_notifyCallback)(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify);
}; // BLERemoteCharacteristic
#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLEREMOTECHARACTERISTIC_H_ */
