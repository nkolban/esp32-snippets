/*
 * BLECharacteristic.h
 *
 *  Created on: Mar 26, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLEXXXCHARACTERISTIC_H_
#define COMPONENTS_CPP_UTILS_BLEXXXCHARACTERISTIC_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <esp_gattc_api.h>




class BLECharacteristicXXX {
public:
	BLECharacteristicXXX(
		uint16_t conn_id,
		esp_gatt_srvc_id_t srvc_id,
		esp_gatt_id_t char_id,
		esp_gatt_char_prop_t char_prop );
	virtual ~BLECharacteristicXXX();

	esp_gatt_srvc_id_t getSrvcId() {
		return m_srvc_id;
	}

	esp_gatt_id_t getCharId() {
		return m_char_id;
	}

	bool isAuth() {
		return m_char_prop & ESP_GATT_CHAR_PROP_BIT_AUTH;
	}
	bool isBroadcast() {
		return m_char_prop & ESP_GATT_CHAR_PROP_BIT_BROADCAST;
	}
	bool isExtProp() {
		return m_char_prop & ESP_GATT_CHAR_PROP_BIT_EXT_PROP;
	}
	bool isIndicate() {
		return m_char_prop & ESP_GATT_CHAR_PROP_BIT_INDICATE;
	}
	bool isNotify() {
		return m_char_prop & ESP_GATT_CHAR_PROP_BIT_NOTIFY;
	}
	bool isRead() {
		return m_char_prop & ESP_GATT_CHAR_PROP_BIT_READ;
	}

	bool isWrite() {
		return m_char_prop & ESP_GATT_CHAR_PROP_BIT_WRITE;
	}
	bool isWrite_NR() {
		return m_char_prop & ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
	}

	void nextCharacterisic(esp_gatt_if_t gattc_if);

private:
	esp_gatt_char_prop_t m_char_prop;
	uint16_t m_conn_id;
	esp_gatt_srvc_id_t m_srvc_id;

	// Contains esp_bt_uuid_t uuid and uint8_t inst_id
	esp_gatt_id_t m_char_id;
};
#endif // CONFIG_BT_ENABLED
#endif /* COMPONENTS_CPP_UTILS_BLEXXXCHARACTERISTIC_H_ */
