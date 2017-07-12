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
#include "FreeRTOS.h"

class BLERemoteService;

class BLERemoteCharacteristic {
public:
	BLERemoteCharacteristic(esp_gatt_id_t charId, esp_gatt_char_prop_t charProp, BLERemoteService *pRemoteService);
	virtual ~BLERemoteCharacteristic();

	// Public member functions
	std::string readValue(void);
	uint8_t     readUInt8(void);
	uint16_t    readUInt16(void);
	uint32_t    readUInt32(void);
	void        registerForNotify(void);
	void        writeValue(std::string newValue, bool response = false);
	void        writeValue(uint8_t newValue, bool response = false);
	std::string toString(void);

private:
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
	std::string m_value;
}; // BLERemoteCharacteristic
#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLEREMOTECHARACTERISTIC_H_ */
