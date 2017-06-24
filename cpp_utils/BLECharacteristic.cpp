/*
 * BLECharacteristic.cpp
 *
 *  Created on: Jun 22, 2017
 *      Author: kolban
 */

#include <sstream>
#include <string.h>
#include <iomanip>
#include <stdlib.h>
#include "sdkconfig.h"
#include <esp_log.h>
#include <esp_err.h>
#include "BLECharacteristic.h"

static char TAG[] = "BLECharacteristic";

extern "C" {
	char *espToString(esp_err_t value);
}

BLECharacteristic::BLECharacteristic(BLEUUID uuid) {
	m_bleUUID            = uuid;
	m_properties         = 0;
	m_value.attr_value   = (uint8_t *)malloc(ESP_GATT_MAX_ATTR_LEN);
	m_value.attr_len     = 0;
	m_value.attr_max_len = ESP_GATT_MAX_ATTR_LEN;
	m_handle             = 0;
}

BLECharacteristic::~BLECharacteristic() {
	free(m_value.attr_value);
}

/**
 * @brief Set the permission to broadcast.
 * @param [in] value The value of the permission.
 * @return N/A
 */
void BLECharacteristic::setBroadcastPermission(bool value) {
	if (value) {
		m_properties |= ESP_GATT_CHAR_PROP_BIT_BROADCAST;
	} else {
		m_properties &= ~ESP_GATT_CHAR_PROP_BIT_BROADCAST;
	}
}

void BLECharacteristic::setReadPermission(bool value) {
	if (value) {
		m_properties |= ESP_GATT_CHAR_PROP_BIT_READ;
	} else {
		m_properties &= ~ESP_GATT_CHAR_PROP_BIT_READ;
	}
}

void BLECharacteristic::setWritePermission(bool value) {
	if (value) {
		m_properties |= ESP_GATT_CHAR_PROP_BIT_WRITE;
	} else {
		m_properties &= ~ESP_GATT_CHAR_PROP_BIT_WRITE;
	}
}

void BLECharacteristic::setNotifyPermission(bool value) {
	if (value) {
		m_properties |= ESP_GATT_CHAR_PROP_BIT_NOTIFY;
	} else {
		m_properties &= ~ESP_GATT_CHAR_PROP_BIT_NOTIFY;
	}
}

void BLECharacteristic::setIndicatePermission(bool value) {
	if (value) {
		m_properties |= ESP_GATT_CHAR_PROP_BIT_INDICATE;
	} else {
		m_properties &= ~ESP_GATT_CHAR_PROP_BIT_INDICATE;
	}
}

void BLECharacteristic::setWriteNoResponsePermission(bool value) {
	if (value) {
		m_properties |= ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
	} else {
		m_properties &= ~ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
	}
}


/**
 * @brief Return a string representation of the characteristic.
 * @return A string representation of the characteristic.
 */
std::string BLECharacteristic::toString() {
	std::stringstream stringstream;
	stringstream << std::hex << std::setfill('0');
	stringstream << "UUID: " << m_bleUUID.toString() + ", handle: 0x" << std::setw(2) << m_handle;
	stringstream <<
			((m_properties & ESP_GATT_CHAR_PROP_BIT_READ)?"Read ":"") <<
			((m_properties & ESP_GATT_CHAR_PROP_BIT_WRITE)?"Write ":"") <<
			((m_properties & ESP_GATT_CHAR_PROP_BIT_WRITE_NR)?"WriteNoResponse ":"") <<
			((m_properties & ESP_GATT_CHAR_PROP_BIT_BROADCAST)?"Broadcast ":"") <<
			((m_properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY)?"Notify ":"") <<
			((m_properties & ESP_GATT_CHAR_PROP_BIT_INDICATE)?"Indicate ":"") <<
			"\n";
	return stringstream.str();
} // toString


BLEUUID BLECharacteristic::getUUID() {
	return m_bleUUID;
}


/**
 * @brief Set the value of the characteristic.
 * @param [in] data The data to set for the characteristic.
 * @param [in] length The length of the data in bytes.
 */
void BLECharacteristic::setValue(uint8_t* data, size_t length) {
	if (length > ESP_GATT_MAX_ATTR_LEN) {
		ESP_LOGE(TAG, "Size %d too large, must be no bigger than %d", length, ESP_GATT_MAX_ATTR_LEN);
		return;
	}
	m_value.attr_len = length;
	memcpy(m_value.attr_value, data, length);
} // setValue


void BLECharacteristic::setValue(std::string value) {
	setValue((uint8_t *)value.data(), value.length());
}

uint8_t* BLECharacteristic::getValue() {
	return m_value.attr_value;
}

size_t BLECharacteristic::getLength() {
	return m_value.attr_len;
}

esp_gatt_char_prop_t BLECharacteristic::getProperties() {
	return m_properties;
}

void BLECharacteristic::setHandle(uint16_t handle) {
	ESP_LOGD(TAG, "Setting handle to be 0x%.2x", handle);
	m_handle = handle;
}

void BLECharacteristic::handleGATTServerEvent(
		esp_gatts_cb_event_t      event,
		esp_gatt_if_t             gatts_if,
		esp_ble_gatts_cb_param_t *param) {
	switch(event) {
		// ESP_GATTS_WRITE_EVT - A request to write the value of a characteristic has arrived.
		//
		// write:
		// - uint16_t conn_id
		// - uint16_t trans_id
		// - esp_bd_addr_t bda
		// - uint16_t handle
		// - uint16_t offset
		// - bool need_rsp
		// - bool is_prep
		// - uint16_t len
		// - uint8_t *value
		case ESP_GATTS_WRITE_EVT: {
			if (param->write.handle == m_handle) {
				setValue(param->write.value, param->write.len);
				esp_gatt_rsp_t rsp;
				rsp.attr_value.len    = getLength();
				rsp.attr_value.handle = m_handle;
				rsp.attr_value.offset = 0;
				rsp.attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
				memcpy(rsp.attr_value.value, getValue(), rsp.attr_value.len);
				esp_err_t errRc = ::esp_ble_gatts_send_response(
						gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, &rsp);
				if (errRc != ESP_OK) {
					ESP_LOGE(TAG, "esp_ble_gatts_send_response: rc=%d %s", errRc, espToString(errRc));
				}
			}
			break;
		} // ESP_GATTS_WRITE_EVT

		// ESP_GATTS_READ_EVT - A request to read the value of a characteristic has arrived.
		//
		// read:
		// - uint16_t conn_id
		// - uint32_t trans_id
		// - esp_bd_addr_t bda
		// - uint16_t handle
		// - uint16_t offset
		// - bool is_long
		// - bool need_rsp
		//
		case ESP_GATTS_READ_EVT: {
			ESP_LOGD(TAG, "- Testing: %d == %d", param->read.handle, m_handle);
			if (param->read.handle == m_handle) {
				ESP_LOGD(TAG, "Sending a response (esp_ble_gatts_send_response)");
				if (param->read.need_rsp) {
					esp_gatt_rsp_t rsp;
					rsp.attr_value.len    = getLength();
					rsp.attr_value.handle = param->read.handle;
					rsp.attr_value.offset = 0;
					rsp.attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
					memcpy(rsp.attr_value.value, getValue(), rsp.attr_value.len);
					esp_err_t errRc = ::esp_ble_gatts_send_response(
							gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, &rsp);
					if (errRc != ESP_OK) {
						ESP_LOGE(TAG, "esp_ble_gatts_send_response: rc=%d %s", errRc, espToString(errRc));
					}
				}
			} // ESP_GATTS_READ_EVT
			break;
		} // ESP_GATTS_READ_EVT
		default: {
			break;
		}
	}// switch event
}
