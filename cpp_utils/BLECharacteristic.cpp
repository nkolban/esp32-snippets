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
#include "BLEService.h"

static char LOG_TAG[] = "BLECharacteristic";

extern "C" {
	char *espToString(esp_err_t value);
}

/**
 * @brief Construct a characteristic
 * @param [in] uuid - UUID for the characteristic.
 * @param [in] properties - Properties for the characteristic.
 */
BLECharacteristic::BLECharacteristic(BLEUUID uuid, uint32_t properties) {
	m_bleUUID            = uuid;
	m_value.attr_value   = (uint8_t *)malloc(ESP_GATT_MAX_ATTR_LEN);
	m_value.attr_len     = 0;
	m_value.attr_max_len = ESP_GATT_MAX_ATTR_LEN;
	m_handle             = 0;
	m_properties         = 0;

	setBroadcastProperty((properties & PROPERTY_BROADCAST) !=0);
	setReadProperty((properties & PROPERTY_READ) !=0);
	setWriteProperty((properties & PROPERTY_WRITE) !=0);
	setNotifyProperty((properties & PROPERTY_NOTIFY) !=0);
	setIndicateProperty((properties & PROPERTY_INDICATE) !=0);
	setWriteNoResponseProperty((properties & PROPERTY_WRITE_NR) !=0);
} // BLECharacteristic


BLECharacteristic::~BLECharacteristic() {
	free(m_value.attr_value);
}


/**
 * @brief Associate a descriptor with this characteristic.
 * @param [in] pDescriptor
 * @return N/A.
 */
void BLECharacteristic::addDescriptor(BLEDescriptor* pDescriptor) {
	ESP_LOGD(LOG_TAG, ">> addDescriptor(): Adding %s to %s", pDescriptor->toString().c_str(), toString().c_str());
	m_descriptorMap.setByUUID(pDescriptor->getUUID(), pDescriptor);
	ESP_LOGD(LOG_TAG, "<< addDescriptor()");
} // addDescriptor


uint16_t BLECharacteristic::getHandle() {
	return m_handle;
} // getHandle


size_t BLECharacteristic::getLength() {
	return m_value.attr_len;
} // getLength


esp_gatt_char_prop_t BLECharacteristic::getProperties() {
	return m_properties;
} // getProperties


BLEUUID BLECharacteristic::getUUID() {
	return m_bleUUID;
} // getUUID


uint8_t* BLECharacteristic::getValue() {
	return m_value.attr_value;
} // getValue


void BLECharacteristic::handleGATTServerEvent(
		esp_gatts_cb_event_t      event,
		esp_gatt_if_t             gatts_if,
		esp_ble_gatts_cb_param_t *param) {
	switch(event) {

		// ESP_GATTS_WRITE_EVT - A request to write the value of a characteristic has arrived.
		//
		// write:
		// - uint16_t      conn_id
		// - uint16_t      trans_id
		// - esp_bd_addr_t bda
		// - uint16_t      handle
		// - uint16_t      offset
		// - bool          need_rsp
		// - bool          is_prep
		// - uint16_t      len
		// - uint8_t      *value
		//
		case ESP_GATTS_WRITE_EVT: {
			if (param->write.handle == m_handle) {
				setValue(param->write.value, param->write.len);
				esp_gatt_rsp_t rsp;
				rsp.attr_value.len      = getLength();
				rsp.attr_value.handle   = m_handle;
				rsp.attr_value.offset   = 0;
				rsp.attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
				memcpy(rsp.attr_value.value, getValue(), rsp.attr_value.len);
				esp_err_t errRc = ::esp_ble_gatts_send_response(
						gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, &rsp);
				if (errRc != ESP_OK) {
					ESP_LOGE(LOG_TAG, "esp_ble_gatts_send_response: rc=%d %s", errRc, espToString(errRc));
				}
			}
			break;
		} // ESP_GATTS_WRITE_EVT

		// ESP_GATTS_READ_EVT - A request to read the value of a characteristic has arrived.
		//
		// read:
		// - uint16_t      conn_id
		// - uint32_t      trans_id
		// - esp_bd_addr_t bda
		// - uint16_t      handle
		// - uint16_t      offset
		// - bool          is_long
		// - bool          need_rsp
		//
		case ESP_GATTS_READ_EVT: {
			ESP_LOGD(LOG_TAG, "- Testing: 0x%.2x == 0x%.2x", param->read.handle, m_handle);
			if (param->read.handle == m_handle) {
				ESP_LOGD(LOG_TAG, "Sending a response (esp_ble_gatts_send_response)");
				if (param->read.need_rsp) {
					esp_gatt_rsp_t rsp;
					rsp.attr_value.len      = getLength();
					rsp.attr_value.handle   = param->read.handle;
					rsp.attr_value.offset   = 0;
					rsp.attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
					memcpy(rsp.attr_value.value, getValue(), rsp.attr_value.len);
					esp_err_t errRc = ::esp_ble_gatts_send_response(
							gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, &rsp);
					if (errRc != ESP_OK) {
						ESP_LOGE(LOG_TAG, "esp_ble_gatts_send_response: rc=%d %s", errRc, espToString(errRc));
					}
				}
			} // ESP_GATTS_READ_EVT
			break;
		} // ESP_GATTS_READ_EVT

		default: {
			break;
		} // default

	} // switch event

	// Give each of the descriptors associated with this characteristic the opportunity to handle the
	// event.
	BLEDescriptor *pDescriptor = m_descriptorMap.getFirst();
	while(pDescriptor != nullptr) {
		pDescriptor->handleGATTServerEvent(event, gatts_if, param);
		pDescriptor = m_descriptorMap.getNext();
	}

} // handleGATTServerEvent


/**
 * @brief Set the permission to broadcast.
 * @param [in] value The value of the property.
 * @return N/A
 */
void BLECharacteristic::setBroadcastProperty(bool value) {
	//ESP_LOGD(LOG_TAG, "setBroadcastProperty(%d)", value);
	if (value) {
		m_properties |= ESP_GATT_CHAR_PROP_BIT_BROADCAST;
	} else {
		m_properties &= ~ESP_GATT_CHAR_PROP_BIT_BROADCAST;
	}
} // setBroadcastProperty


void BLECharacteristic::setHandle(uint16_t handle) {
	ESP_LOGD(LOG_TAG, ">> setHandle(0x%.2x): Setting handle to be 0x%.2x", handle, handle);
	m_handle = handle;
	ESP_LOGD(LOG_TAG, "<< setHandle()");
} // setHandle


void BLECharacteristic::setIndicateProperty(bool value) {
	//ESP_LOGD(LOG_TAG, "setIndicateProperty(%d)", value);
	if (value) {
		m_properties |= ESP_GATT_CHAR_PROP_BIT_INDICATE;
	} else {
		m_properties &= ~ESP_GATT_CHAR_PROP_BIT_INDICATE;
	}
} // setIndicateProperty


void BLECharacteristic::setNotifyProperty(bool value) {
	//ESP_LOGD(LOG_TAG, "setNotifyProperty(%d)", value);
	if (value) {
		m_properties |= ESP_GATT_CHAR_PROP_BIT_NOTIFY;
	} else {
		m_properties &= ~ESP_GATT_CHAR_PROP_BIT_NOTIFY;
	}
} // setNotifyProperty


void BLECharacteristic::setReadProperty(bool value) {
	//ESP_LOGD(LOG_TAG, "setReadProperty(%d)", value);
	if (value) {
		m_properties |= ESP_GATT_CHAR_PROP_BIT_READ;
	} else {
		m_properties &= ~ESP_GATT_CHAR_PROP_BIT_READ;
	}
} // setReadProperty


/**
 * @brief Set the value of the characteristic.
 * @param [in] data The data to set for the characteristic.
 * @param [in] length The length of the data in bytes.
 */
void BLECharacteristic::setValue(uint8_t* data, size_t length) {
	if (length > ESP_GATT_MAX_ATTR_LEN) {
		ESP_LOGE(LOG_TAG, "Size %d too large, must be no bigger than %d", length, ESP_GATT_MAX_ATTR_LEN);
		return;
	}
	m_value.attr_len = length;
	memcpy(m_value.attr_value, data, length);
} // setValue


void BLECharacteristic::setValue(std::string value) {
	setValue((uint8_t *)value.data(), value.length());
} // setValue


void BLECharacteristic::setWriteNoResponseProperty(bool value) {
	//ESP_LOGD(LOG_TAG, "setWriteNoResponseProperty(%d)", value);
	if (value) {
		m_properties |= ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
	} else {
		m_properties &= ~ESP_GATT_CHAR_PROP_BIT_WRITE_NR;
	}
} // setWriteNoResponseProperty


void BLECharacteristic::setWriteProperty(bool value) {
	//ESP_LOGD(LOG_TAG, "setWriteProperty(%d)", value);
	if (value) {
		m_properties |= ESP_GATT_CHAR_PROP_BIT_WRITE;
	} else {
		m_properties &= ~ESP_GATT_CHAR_PROP_BIT_WRITE;
	}
} // setWriteProperty


/**
 * @brief Return a string representation of the characteristic.
 * @return A string representation of the characteristic.
 */
std::string BLECharacteristic::toString() {
	std::stringstream stringstream;
	stringstream << std::hex << std::setfill('0');
	stringstream << "UUID: " << m_bleUUID.toString() + ", handle: 0x" << std::setw(2) << m_handle;
	stringstream << " " <<
		((m_properties & ESP_GATT_CHAR_PROP_BIT_READ)?"Read ":"") <<
		((m_properties & ESP_GATT_CHAR_PROP_BIT_WRITE)?"Write ":"") <<
		((m_properties & ESP_GATT_CHAR_PROP_BIT_WRITE_NR)?"WriteNoResponse ":"") <<
		((m_properties & ESP_GATT_CHAR_PROP_BIT_BROADCAST)?"Broadcast ":"") <<
		((m_properties & ESP_GATT_CHAR_PROP_BIT_NOTIFY)?"Notify ":"") <<
		((m_properties & ESP_GATT_CHAR_PROP_BIT_INDICATE)?"Indicate ":"");
	return stringstream.str();
} // toString

BLEService* BLECharacteristic::getService() {
	return m_pService;
}


/**
 * @brief Register a new characteristic with the ESP runtime.
 * @param [in] pService The service with which to associate this characteristic.
 */
void BLECharacteristic::executeCreate(BLEService* pService) {
	ESP_LOGD(LOG_TAG, ">> executeCreate()");

	if (m_handle != 0) {
		ESP_LOGE(LOG_TAG, "Characteristic already has a handle.");
		return;
	}

	m_pService = pService; // Save the service for this characteristic.

	ESP_LOGD(LOG_TAG, "Registering characteristic (esp_ble_gatts_add_char): uuid: %s, service: %s",
		getUUID().toString().c_str(),
		m_pService->toString().c_str());

	//m_serializeMutex.take("addCharacteristic"); // Take the mutex, released by event ESP_GATTS_ADD_CHAR_EVT
	esp_err_t errRc = ::esp_ble_gatts_add_char(
		m_pService->getHandle(),
		getUUID().getNative(),
		(esp_gatt_perm_t)(ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE),
		getProperties(),
		&m_value,
		NULL);

	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "<< esp_ble_gatts_add_char: rc=%d %s", errRc, espToString(errRc));
		return;
	}

	// Now that we have registered the characteristic, we must also register all the descriptors associated with this
	// characteristic.  We iterate through each of those and invoke the registration call to register them with the
	// ESP environment.

	BLEDescriptor *pDescriptor = m_descriptorMap.getFirst();

	while (pDescriptor != nullptr) {
		pDescriptor->executeCreate(this);
		pDescriptor = m_descriptorMap.getNext();
	} // End while

	ESP_LOGD(LOG_TAG, "<< executeCreate()");
} // executeCreate
