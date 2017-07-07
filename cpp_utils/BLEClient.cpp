/*
 * BLEDevice.cpp
 *
 *  Created on: Mar 22, 2017
 *      Author: kolban
 */
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <esp_log.h>
#include <bt.h>
#include <esp_bt_main.h>
#include <esp_gap_ble_api.h>
#include <esp_gattc_api.h>
#include "BLEClient.h"
#include "BLEUtils.h"
#include "BLEService.h"
#include "GeneralUtils.h"
#include <string>
#include <unordered_set>

static char tag[] = "BLEDevice";

BLEClient::BLEClient() {
	m_deviceType          = 0;
	m_pClientCallbacks    = nullptr;


	m_manufacturerType[0] = 0;
	m_manufacturerType[1] = 0;
	m_conn_id             = 0;
	m_oncharacteristic    = nullptr;
	m_onconnected         = nullptr;
	m_onread              = nullptr;
	m_onsearchcomplete    = nullptr;
	m_gattc_if            = 0;
	m_haveAdvertizement   = false;
}

BLEClient::~BLEClient() {
	ESP_LOGD(tag, "BLEClient object destroyed");
}

/**
 * @brief Connect to the partner.
 * @param [in] address The address of the partner.
 */
void BLEClient::connect(BLEAddress address) {
	m_address = address;
	m_gattc_if = ESP_GATT_IF_NONE;
	esp_err_t errRc = ::esp_ble_gattc_open(
		m_gattc_if,
		*m_address.getNative(), // address
		1                       // direct connection
	);
	if (errRc != ESP_OK) {
		ESP_LOGE(tag, "esp_ble_gattc_open: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}
} // connect


/**
 * @brief Given a UUID, retrieve the corresponding service (assuming it exists).
 */
BLEService BLEClient::findServiceByUUID(esp_bt_uuid_t uuid) {
	assert(uuid.len == ESP_UUID_LEN_16 || uuid.len == ESP_UUID_LEN_32 || uuid.len == ESP_UUID_LEN_128);
	ESP_LOGD(tag, "Looking for service with uuid: %s", BLEUUID(uuid).toString().c_str());
	return m_gattServices.at(uuid);
} // findServiceByUUID

void BLEClient::readCharacteristic(esp_gatt_srvc_id_t srvcId,
		esp_gatt_id_t characteristicId) {
	esp_err_t errRc = esp_ble_gattc_read_char(m_gattc_if, m_conn_id, &srvcId, &characteristicId, ESP_GATT_AUTH_REQ_NONE);
	if (errRc != ESP_OK) {
		ESP_LOGE(tag, "esp_ble_gattc_read_char: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}
}

void BLEClient::readCharacteristic(uint16_t srvcId, uint16_t characteristicId) {
	readCharacteristic(BLEUtils::buildGattSrvcId(BLEUtils::buildGattId(BLEUtils::buildUUID(srvcId))),
			BLEUtils::buildGattId(BLEUtils::buildUUID(characteristicId)));
}


void BLEClient::addService(esp_gatt_srvc_id_t srvc_id) {
	ESP_LOGD(tag, ">> addService: %s", BLEUUID(srvc_id.id.uuid).toString().c_str());
	//BLEService service;
	//service.setService(srvc_id);
	//m_gattServices.insert(std::pair<esp_bt_uuid_t, BLEService>(srvc_id.id.uuid, service));
} // addService


/**
 * @brief Dump the status of this BLE device.
 */
void BLEClient::dump() {
	ESP_LOGD(tag, "--- BLEDeviceDump (this=0x%x)", (uint32_t)this);
	if (!m_haveAdvertizement) {
		ESP_LOGD(tag, "No advertizement data");
	} else {
		ESP_LOGD(tag, "address: %s", m_address.toString().c_str());
		ESP_LOGD(tag, "Manufacturer type: 0x%.2x 0x%.2x", m_manufacturerType[0], m_manufacturerType[1]);
		ESP_LOGD(tag, "Num services: %d", m_services.size());
		if (m_services.size() > 0) {
			for (auto i : m_services) {
				switch(i.length()) {
				case 2:
					ESP_LOGD(tag, "service: %.2x", *(uint16_t *)i.data());
					break;
				case 4:
					ESP_LOGD(tag, "service: %.4x", *(uint32_t *)i.data());
					break;
				case 16:
					ESP_LOGD(tag, "service: %.4x%.4x%.4x%.4x", *(uint32_t *)i.data(), *(uint32_t *)(i.data()+4), *(uint32_t *)(i.data()+8), *(uint32_t *)(i.data()+12));
					break;
				}
			}
		}
	}
	ESP_LOGD(tag, "OnConnected callback: 0x%x", (uint32_t)m_onconnected);
	ESP_LOGD(tag, "OnSearchComplete callback: 0x%x", (uint32_t)m_onsearchcomplete);
	ESP_LOGD(tag, "Connection id: %d", m_conn_id);
	ESP_LOGD(tag, "GATT Client Interface: %d", m_gattc_if);

	// Dump the discovered services by iterating through the map of services.
	for (auto &myPair : m_gattServices) {
		// first: esp_bt_uiid_t, second: BLEService
		myPair.second.dump();
	}
} // dump



/**
 * Retrieve the characteristics for the device service.
 */
void BLEClient::getCharacteristics(esp_gatt_srvc_id_t *srvc_id, esp_gatt_id_t *lastCharacteristic) {
	ESP_LOGD(tag, ">> BLERemoteDevice::getCharacteristics");
	esp_err_t errRc = esp_ble_gattc_get_characteristic(
		m_gattc_if,
		m_conn_id,
		srvc_id,
		lastCharacteristic // Start characteristic
	);
	if (errRc != ESP_OK) {
		ESP_LOGE(tag, "esp_ble_gattc_get_characteristic: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}
	ESP_LOGD(tag, "<< BLEDevice::getCharacteristics");
} // getCharacteristics


void BLEClient::getCharacteristics(BLEService service) {
	// FIX
	/*
	esp_gatt_srvc_id_t tempService = service.getService();
	getCharacteristics(&tempService, nullptr);
	*/
} // getCharacteristics


void BLEClient::getCharacteristics(BLECharacteristicXXX characteristic) {
	esp_gatt_srvc_id_t srvc_id = characteristic.getSrvcId();
	esp_gatt_id_t lastCharacteristic = characteristic.getCharId();
	getCharacteristics(&srvc_id, &lastCharacteristic);
} // getCharacteristics


void BLEClient::getDescriptors() {
}


void BLEClient::searchService() {
	ESP_LOGD(tag, ">> BLEDevice::searchService");
	esp_err_t errRc = esp_ble_gattc_search_service(
		m_gattc_if,
		m_conn_id,
		NULL // Filter UUID
	);
	if (errRc != ESP_OK) {
		ESP_LOGE(tag, "esp_ble_gattc_search_service: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}
	ESP_LOGD(tag, "<< BLEDevice::searchService");
} // searchService


void BLEClient::onCharacteristic(BLECharacteristicXXX characteristic) {
	if (m_oncharacteristic != nullptr) {
		m_oncharacteristic(this, characteristic);
	} else {
		ESP_LOGD(tag, "Call to onCharacteristic but no characteristic callback");
	}
} // onCharacteristic


void BLEClient::onConnected(esp_gatt_status_t status) {
	if (m_onconnected != nullptr) {
		m_onconnected(this, status);
	} else {
		ESP_LOGD(tag, "Call to onConnected but no connected callback");
	}
} // onConnected


/**
 * @brief Called when a characteristic has been read.
 *
 * @param data The data read from the partner device.
 */
void BLEClient::onRead(std::string data) {
	m_onread(this, data);
} // onRead


/**
 * @brief Indication that a service search has completed.
 *
 * A service search is complete following a call to BLEDevice::searchService() and all the
 * services have been returned from the device.
 */
void BLEClient::onSearchComplete() {
	if (m_onsearchcomplete != nullptr) {
		m_onsearchcomplete(this);
	} else {
		ESP_LOGD(tag, "Call to onSearchComplete but no search complete callback");
	}
}

void BLEClient::setClientCallbacks(BLEClientCallbacks* pClientCallbacks) {
	m_pClientCallbacks = pClientCallbacks;
}

void BLEClient::gattClientEventHandler(
	esp_gattc_cb_event_t event,
	esp_gatt_if_t gattc_if,
	esp_ble_gattc_cb_param_t *param) {
	switch(event) {
		// ESP_GATTC_OPEN_EVT
		// open:
		// - esp_gatt_status_t status
		// - uint16_t conn_id
		// - esp_bd_addr_t remote_bda
		// - uint16_t mtu
		case ESP_GATTC_OPEN_EVT: {
			m_conn_id = param->open.conn_id;
			if (m_pClientCallbacks != nullptr) {
				m_pClientCallbacks->onConnect(this);
			}
			break;
		}

		default: {
			break;
		}
	}
}

#endif // CONFIG_BT_ENABLED
