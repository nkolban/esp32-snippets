/*
 * BLECharacteristic.cpp
 *
 *  Created on: Mar 26, 2017
 *      Author: kolban
 */

#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <esp_log.h>
#include <esp_err.h>
#include "BLEXXXCharacteristic.h"

extern "C" {
	char *espToString(esp_err_t value);
}

static char tag[] = "BLECharacteristic";

BLECharacteristicXXX::BLECharacteristicXXX(
	uint16_t conn_id,
	esp_gatt_srvc_id_t srvc_id,
	esp_gatt_id_t char_id,
	esp_gatt_char_prop_t char_prop ) {
	m_conn_id = conn_id;
	m_srvc_id = srvc_id;
	m_char_id = char_id;
	m_char_prop = char_prop;
}

BLECharacteristicXXX::~BLECharacteristicXXX() {
}

void BLECharacteristicXXX::nextCharacterisic(esp_gatt_if_t gattc_if) {
	esp_err_t errRc = esp_ble_gattc_get_characteristic(
		gattc_if,
		m_conn_id,
		&m_srvc_id,
		&m_char_id
	);
	if (errRc != ESP_OK) {
		ESP_LOGE(tag, "esp_ble_gattc_get_characteristic: rc=%d %s", errRc, espToString(errRc));
		return;
	}
}
#endif // CONFIG_BT_ENABLED
