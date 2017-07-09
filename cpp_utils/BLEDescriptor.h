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
class BLEService;
class BLECharacteristic;

class BLEDescriptor {
public:
	BLEDescriptor(BLEUUID uuid);
	virtual ~BLEDescriptor();

	size_t getLength();
	BLEUUID getUUID();
	uint8_t *getValue();
	void handleGATTServerEvent(
			esp_gatts_cb_event_t      event,
			esp_gatt_if_t             gatts_if,
			esp_ble_gatts_cb_param_t *param);
	void setValue(uint8_t *data, size_t size);
	void setValue(std::string value);
	std::string toString();

private:
	friend class BLEDescriptorMap;
	friend class BLECharacteristic;
	BLEUUID m_bleUUID;
	esp_attr_value_t     m_value;
	uint16_t             m_handle;
	BLECharacteristic   *m_pCharacteristic;
	void executeCreate(BLECharacteristic *pCharacteristic);
	uint16_t getHandle();
	void setHandle(uint16_t handle);
};
#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLEDESCRIPTOR_H_ */
