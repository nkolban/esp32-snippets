/*
 * BLEDescriptor.h
 *
 *  Created on: Jun 22, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLEDESCRIPTOR_H_
#define COMPONENTS_CPP_UTILS_BLEDESCRIPTOR_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <string>
#include "BLEUUID.h"
#include "BLECharacteristic.h"
#include <esp_gatts_api.h>
#include "FreeRTOS.h"

class BLEService;
class BLECharacteristic;
class BLEDescriptorCallbacks;

/**
 * @brief A model of a %BLE descriptor.
 */
class BLEDescriptor {
public:
	BLEDescriptor(const char* uuid);
	BLEDescriptor(BLEUUID uuid);
	virtual ~BLEDescriptor();

	uint16_t getHandle();                                   // Get the handle of the descriptor.
	size_t   getLength();                                   // Get the length of the value of the descriptor.
	BLEUUID  getUUID();                                     // Get the UUID of the descriptor.
	uint8_t* getValue();                                    // Get a pointer to the value of the descriptor.
	void handleGATTServerEvent(
			esp_gatts_cb_event_t      event,
			esp_gatt_if_t             gatts_if,
			esp_ble_gatts_cb_param_t* param);

	void setAccessPermissions(esp_gatt_perm_t perm);	      // Set the permissions of the descriptor.
	void setCallbacks(BLEDescriptorCallbacks* pCallbacks);  // Set callbacks to be invoked for the descriptor.
	void setValue(uint8_t* data, size_t size);              // Set the value of the descriptor as a pointer to data.
	void setValue(std::string value);                       // Set the value of the descriptor as a data buffer.

	std::string toString();                                 // Convert the descriptor to a string representation.

private:
	friend class BLEDescriptorMap;
	friend class BLECharacteristic;
	BLEUUID                 m_bleUUID;
	uint16_t                m_handle;
	BLEDescriptorCallbacks* m_pCallback;
	BLECharacteristic*      m_pCharacteristic;
	esp_gatt_perm_t				  m_permissions = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE;
	FreeRTOS::Semaphore     m_semaphoreCreateEvt = FreeRTOS::Semaphore("CreateEvt");
	esp_attr_value_t        m_value;

	void executeCreate(BLECharacteristic* pCharacteristic);
	void setHandle(uint16_t handle);
}; // BLEDescriptor


/**
 * @brief Callbacks that can be associated with a %BLE descriptors to inform of events.
 *
 * When a server application creates a %BLE descriptor, we may wish to be informed when there is either
 * a read or write request to the descriptors value.  An application can register a
 * sub-classed instance of this class and will be notified when such an event happens.
 */
class BLEDescriptorCallbacks {
public:
	virtual ~BLEDescriptorCallbacks();
	virtual void onRead(BLEDescriptor* pDescriptor);
	virtual void onWrite(BLEDescriptor* pDescriptor);
};
#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLEDESCRIPTOR_H_ */
