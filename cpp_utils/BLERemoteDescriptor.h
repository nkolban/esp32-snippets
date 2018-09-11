/*
 * BLERemoteDescriptor.h
 *
 *  Created on: Jul 8, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLEREMOTEDESCRIPTOR_H_
#define COMPONENTS_CPP_UTILS_BLEREMOTEDESCRIPTOR_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <string>

#include <esp_gattc_api.h>

#include "BLERemoteCharacteristic.h"
#include "BLEUUID.h"
#include "FreeRTOS.h"

class BLERemoteCharacteristic;
/**
 * @brief A model of remote %BLE descriptor.
 */
class BLERemoteDescriptor {
public:
	uint16_t    getHandle();
	BLERemoteCharacteristic* getRemoteCharacteristic();
	BLEUUID     getUUID();
	std::string readValue(void);
	uint8_t     readUInt8(void);
	uint16_t    readUInt16(void);
	uint32_t    readUInt32(void);
	std::string toString(void);
	void        writeValue(uint8_t* data, size_t length, bool response = false);
	void        writeValue(std::string newValue, bool response = false);
	void        writeValue(uint8_t newValue, bool response = false);


private:
	friend class BLERemoteCharacteristic;
	BLERemoteDescriptor(
		uint16_t                 handle,
		BLEUUID                  uuid,
		BLERemoteCharacteristic* pRemoteCharacteristic
	);
	uint16_t                 m_handle;                  // Server handle of this descriptor.
	BLEUUID                  m_uuid;                    // UUID of this descriptor.
	std::string              m_value;                   // Last received value of the descriptor.
	BLERemoteCharacteristic* m_pRemoteCharacteristic;   // Reference to the Remote characteristic of which this descriptor is associated.
	FreeRTOS::Semaphore      m_semaphoreReadDescrEvt      = FreeRTOS::Semaphore("ReadDescrEvt");


};
#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLEREMOTEDESCRIPTOR_H_ */
