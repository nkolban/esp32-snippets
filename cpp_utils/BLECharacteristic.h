/*
 * BLECharacteristic.h
 *
 *  Created on: Jun 22, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLECHARACTERISTIC_H_
#define COMPONENTS_CPP_UTILS_BLECHARACTERISTIC_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <string>
#include "BLEUUID.h"
#include <esp_gatts_api.h>
#include "BLEDescriptor.h"
#include "BLEDescriptorMap.h"
#include "BLECharacteristicCallbacks.h"

class BLEService;
class BLEDescriptor;
class BLECharacteristicCallbacks;

class BLECharacteristic {
public:
	BLECharacteristic(BLEUUID uuid, uint32_t properties = 0);
	virtual ~BLECharacteristic();

	void     addDescriptor(BLEDescriptor *pDescriptor);
	size_t   getLength();
	BLEUUID  getUUID();
	uint8_t *getValue();

	void indicate();
	void notify();
	void setBroadcastProperty(bool value);
	void setCallbacks(BLECharacteristicCallbacks *pCallbacks);
	void setIndicateProperty(bool value);
	void setNotifyProperty(bool value);
	void setReadProperty(bool value);
	void setValue(uint8_t *data, size_t size);
	void setValue(std::string value);
	void setWriteProperty(bool value);
	void setWriteNoResponseProperty(bool value);
	std::string toString();


	static const uint32_t PROPERTY_READ      = 1<<0;
	static const uint32_t PROPERTY_WRITE     = 1<<1;
	static const uint32_t PROPERTY_NOTIFY    = 1<<2;
	static const uint32_t PROPERTY_BROADCAST = 1<<3;
	static const uint32_t PROPERTY_INDICATE  = 1<<4;
	static const uint32_t PROPERTY_WRITE_NR  = 1<<5;

private:
	friend class BLEServer;
	friend class BLEService;
	friend class BLEDescriptor;
	friend class BLECharacteristicMap;

	BLEUUID              m_bleUUID;
	esp_gatt_char_prop_t m_properties;
	esp_attr_value_t     m_value;
	uint16_t             m_handle;
	BLEService          *m_pService;
	BLEDescriptorMap     m_descriptorMap;
	BLECharacteristicCallbacks *m_pCallbacks;

	void handleGATTServerEvent(
			esp_gatts_cb_event_t      event,
			esp_gatt_if_t             gatts_if,
			esp_ble_gatts_cb_param_t *param);

	void                 executeCreate(BLEService *pService);
	uint16_t             getHandle();
	esp_gatt_char_prop_t getProperties();
	BLEService          *getService();
	void                 setHandle(uint16_t handle);
}; // BLECharacteristic
#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLECHARACTERISTIC_H_ */
