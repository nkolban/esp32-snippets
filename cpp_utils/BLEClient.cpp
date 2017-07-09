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

/*
 * Design
 * ------
 * When we perform a searchService() requests, we are asking the BLE server to return each of the services
 * that it exposes.  For each service, we received an ESP_GATTC_SEARCH_RES_EVT event which contains details
 * of the exposed service including its UUID.
 *
 * The objects we will invent for a BLEClient will be as follows:
 * * BLERemoteService - A model of a remote service.
 * * BLERemoteCharacteristic - A model of a remote characteristic
 * * BLERemoteDescriptor - A model of a remote descriptor.
 *
 * Since there is a hierarchical relationship here, we will have the idea that from a BLERemoteService will own
 * zero or more remote characteristics and a BLERemoteCharacteristic will own zero or more remote BLEDescriptors.
 *
 * We will assume that a BLERemoteService contains a map that maps BLEUUIDs to the set of owned characteristics
 * and that a BLECharacteristic contains a map that maps BLEUUIDs to the set of owned descriptors.
 *
 *
 */
static char LOG_TAG[] = "BLEClient";

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
} // BLEClient

BLEClient::~BLEClient() {
	ESP_LOGD(LOG_TAG, "BLEClient object destroyed");
} // ~BLEClient

/**
 * @brief Connect to the partner.
 * @param [in] address The address of the partner.
 */
void BLEClient::connect(BLEAddress address) {
	ESP_LOGD(LOG_TAG, ">> connect(%s)", address.toString().c_str());
// We need the connection handle that we get from registering the application.  We register the app
// and then block on its completion.  When the event has arrived, we will have the handle.
	m_semaphoreRegEvt.take("connect");
	esp_err_t errRc = esp_ble_gattc_app_register(0);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gattc_app_register: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}
	m_semaphoreRegEvt.take("connect");
	m_semaphoreRegEvt.give();


	m_address = address;
	m_semaphoreOpenEvt.take("connect");
	errRc = ::esp_ble_gattc_open(
		m_gattc_if,
		*m_address.getNative(), // address
		1                       // direct connection
	);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gattc_open: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}
	m_semaphoreOpenEvt.take("connect");
	m_semaphoreOpenEvt.give();
	ESP_LOGD(LOG_TAG, "<< connect()");
} // connect


/**
 * @brief Given a UUID, retrieve the corresponding service (assuming it exists).
 */
BLEService BLEClient::findServiceByUUID(esp_bt_uuid_t uuid) {
	assert(uuid.len == ESP_UUID_LEN_16 || uuid.len == ESP_UUID_LEN_32 || uuid.len == ESP_UUID_LEN_128);
	ESP_LOGD(LOG_TAG, "Looking for service with uuid: %s", BLEUUID(uuid).toString().c_str());
	return m_gattServices.at(uuid);
} // findServiceByUUID

void BLEClient::readCharacteristic(esp_gatt_srvc_id_t srvcId,
		esp_gatt_id_t characteristicId) {
	esp_err_t errRc = esp_ble_gattc_read_char(m_gattc_if, m_conn_id, &srvcId, &characteristicId, ESP_GATT_AUTH_REQ_NONE);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gattc_read_char: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}
}

void BLEClient::readCharacteristic(uint16_t srvcId, uint16_t characteristicId) {
	readCharacteristic(BLEUtils::buildGattSrvcId(BLEUtils::buildGattId(BLEUtils::buildUUID(srvcId))),
			BLEUtils::buildGattId(BLEUtils::buildUUID(characteristicId)));
}


void BLEClient::addService(esp_gatt_srvc_id_t srvc_id) {
	ESP_LOGD(LOG_TAG, ">> addService: %s", BLEUUID(srvc_id.id.uuid).toString().c_str());
	//BLEService service;
	//service.setService(srvc_id);
	//m_gattServices.insert(std::pair<esp_bt_uuid_t, BLEService>(srvc_id.id.uuid, service));
} // addService


/**
 * @brief Dump the status of this BLE device.
 */
void BLEClient::dump() {
	ESP_LOGD(LOG_TAG, "--- BLEDeviceDump (this=0x%x)", (uint32_t)this);
	if (!m_haveAdvertizement) {
		ESP_LOGD(LOG_TAG, "No advertizement data");
	} else {
		ESP_LOGD(LOG_TAG, "address: %s", m_address.toString().c_str());
		ESP_LOGD(LOG_TAG, "Manufacturer type: 0x%.2x 0x%.2x", m_manufacturerType[0], m_manufacturerType[1]);
		ESP_LOGD(LOG_TAG, "Num services: %d", m_services.size());
		if (m_services.size() > 0) {
			for (auto i : m_services) {
				switch(i.length()) {
				case 2:
					ESP_LOGD(LOG_TAG, "service: %.2x", *(uint16_t *)i.data());
					break;
				case 4:
					ESP_LOGD(LOG_TAG, "service: %.4x", *(uint32_t *)i.data());
					break;
				case 16:
					ESP_LOGD(LOG_TAG, "service: %.4x%.4x%.4x%.4x", *(uint32_t *)i.data(), *(uint32_t *)(i.data()+4), *(uint32_t *)(i.data()+8), *(uint32_t *)(i.data()+12));
					break;
				}
			}
		}
	}
	ESP_LOGD(LOG_TAG, "OnConnected callback: 0x%x", (uint32_t)m_onconnected);
	ESP_LOGD(LOG_TAG, "OnSearchComplete callback: 0x%x", (uint32_t)m_onsearchcomplete);
	ESP_LOGD(LOG_TAG, "Connection id: %d", m_conn_id);
	ESP_LOGD(LOG_TAG, "GATT Client Interface: %d", m_gattc_if);

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
	ESP_LOGD(LOG_TAG, ">> BLERemoteDevice::getCharacteristics");
	esp_err_t errRc = esp_ble_gattc_get_characteristic(
		m_gattc_if,
		m_conn_id,
		srvc_id,
		lastCharacteristic // Start characteristic
	);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gattc_get_characteristic: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}
	ESP_LOGD(LOG_TAG, "<< BLEDevice::getCharacteristics");
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
	ESP_LOGD(LOG_TAG, ">> searchService");
	esp_err_t errRc = esp_ble_gattc_search_service(
		m_gattc_if,
		m_conn_id,
		NULL // Filter UUID
	);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gattc_search_service: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}
	ESP_LOGD(LOG_TAG, "<< searchService");
} // searchService


void BLEClient::onCharacteristic(BLECharacteristicXXX characteristic) {
	if (m_oncharacteristic != nullptr) {
		m_oncharacteristic(this, characteristic);
	} else {
		ESP_LOGD(LOG_TAG, "Call to onCharacteristic but no characteristic callback");
	}
} // onCharacteristic


void BLEClient::onConnected(esp_gatt_status_t status) {
	if (m_onconnected != nullptr) {
		m_onconnected(this, status);
	} else {
		ESP_LOGD(LOG_TAG, "Call to onConnected but no connected callback");
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
		ESP_LOGD(LOG_TAG, "Call to onSearchComplete but no search complete callback");
	}
}


/**
 * @brief Set the callbacks that will be invoked.
 */
void BLEClient::setClientCallbacks(BLEClientCallbacks* pClientCallbacks) {
	m_pClientCallbacks = pClientCallbacks;
} // setClientCallbacks



/**
 * @brief Handle GATT Client events
 */
void BLEClient::gattClientEventHandler(
	esp_gattc_cb_event_t      event,
	esp_gatt_if_t             gattc_if,
	esp_ble_gattc_cb_param_t *param) {

	// Execute handler code based on the type of event received.
	switch(event) {
		//
		// ESP_GATTC_OPEN_EVT
		//
		// open:
		// - esp_gatt_status_t status
		// - uint16_t          conn_id
		// - esp_bd_addr_t     remote_bda
		// - uint16_t          mtu
		//
		case ESP_GATTC_OPEN_EVT: {
			m_conn_id = param->open.conn_id;
			if (m_pClientCallbacks != nullptr) {
				m_pClientCallbacks->onConnect(this);
			}
			m_semaphoreOpenEvt.give();
			break;
		} // ESP_GATTC_OPEN_EVT


		//
		// ESP_GATTC_REG_EVT
		//
		// reg:
		// esp_gatt_status_t status
		// uint16_t          app_id
		//
		case ESP_GATTC_REG_EVT: {
			m_gattc_if = gattc_if;
			m_semaphoreRegEvt.give();
			break;
		} // ESP_GATTC_REG_EVT

		//
		// ESP_GATTC_SEARCH_CMPL_EVT
		//
		// search_cmpl:
		// - esp_gatt_status_t status
		// - uint16_t          conn_id
		//
		case ESP_GATTC_SEARCH_CMPL_EVT: {
			m_semaphoreSearchCmplEvt.give();
			break;
		} // ESP_GATTC_SEARCH_CMPL_EVT


		//
		// ESP_GATTC_SEARCH_RES_EVT
		//
		// search_res:
		// - uint16_t           conn_id
		// - esp_gatt_srvc_id_t srvc_id
		//
		case ESP_GATTC_SEARCH_RES_EVT: {
			break;
		} // ESP_GATTC_SEARCH_RES_EVT

		default: {
			break;
		}
	} // Switch
} // gattClientEventHandler


/**
 * @brief Ask the remote BLE server for its services.
 * @return N/A
 */
void BLEClient::getServices() {
/*
 * Design
 * ------
 * We invoke esp_ble_gattc_search_service.  This will request a list of the service exposed by the
 * peer BLE partner to be returned as events.  Each event will be an an instance of ESP_GATTC_SEARCH_RES_EVT
 * and will culminate with an ESP_GATTC_SEARCH_CMPL_EVT when all have been received.
 */
	ESP_LOGD(LOG_TAG, ">> getServices");
	esp_err_t errRc = esp_ble_gattc_search_service(
		m_gattc_if,
		m_conn_id,
		NULL // Filter UUID
	);
	m_semaphoreSearchCmplEvt.take("getServices");
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gattc_search_service: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}
	m_semaphoreSearchCmplEvt.take("getServices");
	m_semaphoreSearchCmplEvt.give();
	ESP_LOGD(LOG_TAG, "<< searchService");
}

#endif // CONFIG_BT_ENABLED
