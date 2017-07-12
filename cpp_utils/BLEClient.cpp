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
#include <sstream>
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
	m_pClientCallbacks = nullptr;
	m_conn_id          = 0;
	m_gattc_if         = 0;
	m_haveServices     = false;
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
		getGattcIf(),
		*getAddress().getNative(), // address
		1                          // direct connection
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
 * @brief Disconnect from the peer.
 * @return N/A.
 */
void BLEClient::disconnect() {
	ESP_LOGD(LOG_TAG, ">> disconnect()");
	esp_err_t errRc = ::esp_ble_gattc_close(getGattcIf(), getConnId());
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gattc_close: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}
	ESP_LOGD(LOG_TAG, "<< disconnect()");
} // disconnect

/**
 * @brief Handle GATT Client events
 */
void BLEClient::gattClientEventHandler(
	esp_gattc_cb_event_t      event,
	esp_gatt_if_t             gattc_if,
	esp_ble_gattc_cb_param_t* evtParam) {

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
			m_conn_id = evtParam->open.conn_id;
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
			BLEUUID uuid = BLEUUID(evtParam->search_res.srvc_id);
			BLERemoteService* pRemoteService = new BLERemoteService(evtParam->search_res.srvc_id, this);
			m_servicesMap.insert(std::pair<std::string, BLERemoteService *>(uuid.toString(), pRemoteService));
			break;
		} // ESP_GATTC_SEARCH_RES_EVT

		default: {
			break;
		}
	} // Switch

	for (auto &myPair : m_servicesMap) {
	   myPair.second->gattClientEventHandler(event, gattc_if, evtParam);
	}

} // gattClientEventHandler


BLEAddress BLEClient::getAddress() {
	return m_address;
} // getAddress


uint16_t BLEClient::getConnId() {
	return m_conn_id;
} // getConnId


esp_gatt_if_t BLEClient::getGattcIf() {
	return m_gattc_if;
} // getGattcIf


/**
 * @brief Get the service object corresponding to the uuid.
 * @param [in] uuid The UUID of the service being sought.
 * @return A reference to the Service or nullptr if don't know about it.
 */
BLERemoteService* BLEClient::getService(BLEUUID uuid) {
// Design
// ------
// We wish to retrieve the service given its UUID.  It is possible that we have not yet asked the
// device what services it has in which case we have nothing to match against.  If we have not
// asked the device about its services, then we do that now.  Once we get the results we can then
// examine the services map to see if it has the service we are looking for.
	if (!m_haveServices) {
		getServices();
	}
	std::string v = uuid.toString();
	for (auto &myPair : m_servicesMap) {
		if (myPair.first == v) {
			return myPair.second;
		}
	}
	return nullptr;
} // getService

/**
 * @brief Ask the remote BLE server for its services.
 * A BLE Server exposes a set of services for its partners.  Here we ask the server for its set of
 * services and wait until we have received them all.
 * @return N/A
 */
std::map<std::string, BLERemoteService*>* BLEClient::getServices() {
/*
 * Design
 * ------
 * We invoke esp_ble_gattc_search_service.  This will request a list of the service exposed by the
 * peer BLE partner to be returned as events.  Each event will be an an instance of ESP_GATTC_SEARCH_RES_EVT
 * and will culminate with an ESP_GATTC_SEARCH_CMPL_EVT when all have been received.
 */
	ESP_LOGD(LOG_TAG, ">> getServices");
	m_servicesMap.empty();
	esp_err_t errRc = esp_ble_gattc_search_service(
		getGattcIf(),
		getConnId(),
		NULL // Filter UUID
	);
	m_semaphoreSearchCmplEvt.take("getServices");
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gattc_search_service: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return &m_servicesMap;
	}
	m_semaphoreSearchCmplEvt.take("getServices");
	m_semaphoreSearchCmplEvt.give();
	m_haveServices = true;
	ESP_LOGD(LOG_TAG, "<< getServices");
	return &m_servicesMap;
} // getServices


/**
 * @brief Set the callbacks that will be invoked.
 */
void BLEClient::setClientCallbacks(BLEClientCallbacks* pClientCallbacks) {
	m_pClientCallbacks = pClientCallbacks;
} // setClientCallbacks



/**
 * @brief Return a string representation of this client.
 * @return A string representation of this client.
 */
std::string BLEClient::toString() {
	std::ostringstream ss;
	ss << "address: " << m_address.toString();
	ss << "\nServices:\n";
	for (auto &myPair : m_servicesMap) {
		ss << myPair.second->toString() << "\n";
	   // myPair.second is the value
	}
	return ss.str();
} // toString

#endif // CONFIG_BT_ENABLED
