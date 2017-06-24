/*
 * BLEService.cpp
 *
 *  Created on: Mar 25, 2017
 *      Author: kolban
 */

// A service is identified by a UUID.  A service is also the container for one or more characteristics.

#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <esp_log.h>
#include <esp_err.h>
#include <esp_gatts_api.h>

#include "BLEService.h"
#include "BLEUtils.h"

extern "C" {
	char *espToString(esp_err_t value);
}

static char tag[] = "BLEService";

BLEService::BLEService(BLEUUID uuid) {
	m_uuid     = uuid;
	m_handle   = 0;
	m_gatts_if = 0;
}


BLEService::~BLEService() {
}


/**
 * @brief Create the service.
 * @param [in] gatts_if The handle of the GATT server interface.
 * @return N/A.
 */
void BLEService::create(esp_gatt_if_t gatts_if) {
	m_gatts_if           = gatts_if;
	m_srvc_id.id.inst_id = 0;
	m_srvc_id.id.uuid    = *m_uuid.getNative();

	ESP_LOGD(tag, "Creating service (esp_ble_gatts_create_service)");
	esp_err_t errRc = ::esp_ble_gatts_create_service(m_gatts_if, &m_srvc_id, 4);
	if (errRc != ESP_OK) {
		ESP_LOGE(tag, "esp_ble_gatts_create_service: rc=%d %s", errRc, espToString(errRc));
		return;
	}
} // create

/**
 * Dump details of this BLE GATT service.
 */
void BLEService::dump() {
	std::string name = "unknown";
	if (m_srvc_id.id.uuid.len == ESP_UUID_LEN_16) {
		name = BLEUtils::gattServiceToString(m_srvc_id.id.uuid.uuid.uuid16);
	}
	ESP_LOGD(tag, "Service: uuid:%s, handle: 0x%.2x",
		m_uuid.toString().c_str(),
		m_handle);
	ESP_LOGD(tag, "Characteristics:\n%s", m_characteristicMap.toString().c_str());
} // dump


void BLEService::setService(esp_gatt_srvc_id_t srvc_id) {
	m_srvc_id = srvc_id;
}

esp_gatt_srvc_id_t BLEService::getService() {
	return m_srvc_id;
}


/**
 * @brief Get the UUID of the service.
 * @return the UUID of the service.
 */
BLEUUID BLEService::getUUID() {
	return m_uuid;
} // getUUID


/**
 * @brief Start the service.
 * @return Start the service.
 */
void BLEService::start() {
	ESP_LOGD(tag, "Starting service (esp_ble_gatts_start_service): handle=0x%.2x", m_handle);
	esp_err_t errRc = ::esp_ble_gatts_start_service(m_handle);
	if (errRc != ESP_OK) {
		ESP_LOGE(tag, "esp_ble_gatts_start_service: rc=%d %s", errRc, espToString(errRc));
		return;
	}
} // start

/**
 * @brief Set the handle associated with this service.
 * @param [in] handle The handle associated with the service.
 */
void BLEService::setHandle(uint16_t handle) {
	m_handle = handle;
} // setHandle


/**
 * @brief Get the handle associated with this service.
 * @return The handle associated with this service.
 */
uint16_t BLEService::getHandle() {
	return m_handle;
}


/**
 * @brief Add a characteristic to the service.
 * @param [in] pCharacteristic A pointer to the characteristic to be added.
 */
void BLEService::addCharacteristic(BLECharacteristic* pCharacteristic) {
// We maintain a mapping of characteristics owned by this service.  These are managed by the
// BLECharacteristicMap class instance found in m_characteristicMap.  We add the characteristic
// to the map and then ask the service to add the characteristic at the BLE level (ESP-IDF).
//
	ESP_LOGD(tag, "Adding characteristic (esp_ble_gatts_add_char): uuid=%s, serviceHandle=0x%.2x",
		pCharacteristic->getUUID().toString().c_str(),
		getHandle());

	// Check that we don't add the same characteristic twice.
	if (m_characteristicMap.getByUUID(pCharacteristic->getUUID()) != nullptr) {
		ESP_LOGE(tag, "Attempt to add a characteristic but we already have one with this UUID");
		return;
	}

	m_characteristicMap.setByUUID(pCharacteristic->getUUID(), pCharacteristic);
	pCharacteristic->m_pService = this;
	ESP_LOGD(tag, "We think we added a characteristic ...");
	dump();

	esp_err_t errRc = ::esp_ble_gatts_add_char(
		getHandle(),
		pCharacteristic->getUUID().getNative(),
		(esp_gatt_perm_t)(ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE),
		pCharacteristic->getProperties(),
		&pCharacteristic->m_value,
		NULL);

	if (errRc != ESP_OK) {
		ESP_LOGE(tag, "esp_ble_gatts_add_char: rc=%d %s", errRc, espToString(errRc));
		return;
	}
} // addCharacteristic

void BLEService::handleGATTServerEvent(
		esp_gatts_cb_event_t      event,
		esp_gatt_if_t             gatts_if,
		esp_ble_gatts_cb_param_t *param) {


	switch(event) {
	  // ESP_GATTS_ADD_CHAR_EVT - Indicate that a characteristic was added to the service.
		// add_char:
		// - esp_gatt_status_t status
		// - uint16_t attr_handle
		// - uint16_t service_handle
		// - esp_bt_uuid_t char_uuid

		// If we have reached the correct service, the locate the characteristic and remember the handle
		// for that characteristic.
		case ESP_GATTS_ADD_CHAR_EVT: {
			if (m_handle == param->add_char.service_handle) {
				BLECharacteristic *pCharacteristic = getCharacteristic(BLEUUID(param->add_char.char_uuid));
				if (pCharacteristic == nullptr) {
					ESP_LOGE(tag, "Expected to find characteristic with UUID: %s, but didnt!",
							BLEUUID(param->add_char.char_uuid).toString().c_str());
					dump();
					break;
				}
				pCharacteristic->setHandle(param->add_char.attr_handle);
				m_characteristicMap.setByHandle(param->add_char.attr_handle, pCharacteristic);
				//ESP_LOGD(tag, "Characteristic map: %s", m_characteristicMap.toString().c_str());
				break;
			} // Reached the correct service.
		} // ESP_GATTS_ADD_CHAR_EVT

		default: {
			break;
		}
	} // Switch

	m_characteristicMap.handleGATTServerEvent(event, gatts_if, param);
}

BLECharacteristic* BLEService::getCharacteristic(BLEUUID uuid) {
	return m_characteristicMap.getByUUID(uuid);
}

#endif // CONFIG_BT_ENABLED
