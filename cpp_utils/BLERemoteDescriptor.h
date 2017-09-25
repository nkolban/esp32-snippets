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
/**
 * @brief A model of remote %BLE descriptor.
 */
class BLERemoteDescriptor {
public:
	BLEUUID     getUUID();
	std::string readValue(void);
	uint8_t     readUInt8(void);
	uint16_t    readUInt16(void);
	uint32_t    readUInt32(void);
	std::string toString(void);
	//void        registerForNotify(void (*notifyCallback)(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify));
	void        writeValue(uint8_t* data, size_t length, bool response = false);
	void        writeValue(std::string newValue, bool response = false);
	void        writeValue(uint8_t newValue, bool response = false);


private:
	uint16_t                 m_handle;
	BLEUUID                  m_uuid;
	std::string              m_value;
	BLERemoteCharacteristic* m_pRemoteCharacteristic;

	uint16_t getHandle();
};
#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLEREMOTEDESCRIPTOR_H_ */
