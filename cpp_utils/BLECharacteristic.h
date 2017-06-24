/*
 * BLECharacteristic.h
 *
 *  Created on: Jun 22, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLECHARACTERISTIC_H_
#define COMPONENTS_CPP_UTILS_BLECHARACTERISTIC_H_
#include <string>
#include "BLEUUID.h"
#include <esp_gatts_api.h>
class BLEService;

class BLECharacteristic {
public:
	BLECharacteristic(BLEUUID uuid);
	virtual ~BLECharacteristic();
	size_t getLength();
	BLEUUID getUUID();
	uint8_t *getValue();
	void setBroadcastPermission(bool value);
	void setIndicatePermission(bool value);
	void setNotifyPermission(bool value);
	void setReadPermission(bool value);
	void setValue(uint8_t *data, size_t size);
	void setValue(std::string value);
	void setWritePermission(bool value);
	void setWriteNoResponsePermission(bool value);
	std::string toString();
	void handleGATTServerEvent(
			esp_gatts_cb_event_t      event,
			esp_gatt_if_t             gatts_if,
			esp_ble_gatts_cb_param_t *param);

private:
	friend class BLEServer;
	friend class BLEService;
	friend class BLECharacteristicMap;
	BLEUUID m_bleUUID;
	esp_gatt_char_prop_t m_properties;
	esp_attr_value_t     m_value;
	uint16_t             m_handle;
	BLEService          *m_pService;
	esp_gatt_char_prop_t getProperties();
	void setHandle(uint16_t handle);
};

#endif /* COMPONENTS_CPP_UTILS_BLECHARACTERISTIC_H_ */
