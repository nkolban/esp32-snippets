/*
 * BLERemoteService.cpp
 *
 *  Created on: Jul 8, 2017
 *      Author: kolban
 */
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include <sstream>
#include "BLERemoteService.h"
#include "BLEUtils.h"
#include "GeneralUtils.h"
#include <esp_log.h>
#include <esp_err.h>

static const char LOG_TAG[] = "BLERemoteService";

BLERemoteService::BLERemoteService(
	esp_gatt_srvc_id_t srvcId,
	BLEClient *pClient) {

	m_srvcId  = srvcId;
	m_pClient = pClient;
	m_uuid    = BLEUUID(m_srvcId);
	m_haveCharacteristics = false;
}


BLERemoteService::~BLERemoteService() {
	removeCharacteristics();
}

static bool compareSrvcId(esp_gatt_srvc_id_t id1, esp_gatt_srvc_id_t id2) {
	if (id1.id.inst_id != id2.id.inst_id) {
		return false;
	}
	if (!BLEUUID(id1.id.uuid).equals(BLEUUID(id2.id.uuid))) {
		return false;
	}
	return true;
} // compareSrvcId


/**
 * @brief Handle GATT Client events
 */
void BLERemoteService::gattClientEventHandler(
	esp_gattc_cb_event_t      event,
	esp_gatt_if_t             gattc_if,
	esp_ble_gattc_cb_param_t *evtParam) {
	switch(event) {
		//
		// ESP_GATTC_GET_CHAR_EVT
		//
		// get_char:
		// - esp_gatt_status_t    status
		// - uin1t6_t             conn_id
		// - esp_gatt_srvc_id_t   srvc_id
		// - esp_gatt_id_t        char_id
		// - esp_gatt_char_prop_t char_prop
		//
		case ESP_GATTC_GET_CHAR_EVT: {
			// Is this event for this service?  If yes, then the local srvc_id and the event srvc_id will be
			// the same.
			if (compareSrvcId(m_srvcId, evtParam->get_char.srvc_id) == false) {
				break;
			}

			if (evtParam->get_char.status != ESP_GATT_OK) {
				m_semaphoreGetCharEvt.give();
				break;
			}

			m_characteristicMap.insert(std::pair<std::string, BLERemoteCharacteristic *>(
					BLEUUID(evtParam->get_char.char_id.uuid).toString(),
					new BLERemoteCharacteristic(evtParam->get_char.char_id, evtParam->get_char.char_prop, this)	));
			::esp_ble_gattc_get_characteristic(
					m_pClient->getGattcIf(),
					m_pClient->getConnId(),
					&m_srvcId,
					&evtParam->get_char.char_id);
			break;
		} // ESP_GATTC_GET_CHAR_EVT

		default: {
			break;
		}
	} // switch

	for (auto &myPair : m_characteristicMap) {
	   myPair.second->gattClientEventHandler(event, gattc_if, evtParam);
	}
} // gattClientEventHandler


/**
 * @brief Get the characteristic object for the UUID.
 * @param [in] uuid Characteristic uuid.
 * @return Reference to the characteristic object.
 */
BLERemoteCharacteristic* BLERemoteService::getCharacteristic(BLEUUID uuid) {
// Design
// ------
// We wish to retrieve the characteritic given its UUID.  It is possible that we have not yet asked the
// device what characteristics it has in which case we have nothing to match against.  If we have not
// asked the device about its characteristics, then we do that now.  Once we get the results we can then
// examine the characteristics map to see if it has the characteristic we are looking for.
	if (!m_haveCharacteristics) {
		getCharacteristics();
	}
	std::string v = uuid.toString();
	for (auto &myPair : m_characteristicMap) {
		if (myPair.first == v) {
			return myPair.second;
		}
	}
	return nullptr;
} // getCharacteristic


/**
 * @brief Retrieve all the characteristics for this service.
 * @return N/A
 */
void BLERemoteService::getCharacteristics() {

	ESP_LOGD(LOG_TAG, ">> getCharacteristics() for service: %s", getUUID().toString().c_str());
	removeCharacteristics();
	m_semaphoreGetCharEvt.take("getCharacteristics");
	esp_err_t errRc = ::esp_ble_gattc_get_characteristic(
		m_pClient->getGattcIf(),
		m_pClient->getConnId(),
		&m_srvcId,
		nullptr);
	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gattc_get_characteristic: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}
	m_semaphoreGetCharEvt.take("getCharacteristics");
	m_semaphoreGetCharEvt.give();
	m_haveCharacteristics = true;
	ESP_LOGD(LOG_TAG, "<< getCharacteristics()");
} // getCharacteristics


BLEClient* BLERemoteService::getClient() {
	return m_pClient;
}

esp_gatt_srvc_id_t* BLERemoteService::getSrvcId() {
	return &m_srvcId;
}

BLEUUID BLERemoteService::getUUID() {
	return m_uuid;
}


/**
 * @brief Delete the characteristics in the characteristics map.
 * We maintain a map called m_characteristicsMap that contains pointers to BLERemoteCharacteristic
 * object references.  Since we allocated these in this class, we are also responsible for deleteing
 * them.  This method does just that.
 * @return N/A.
 */
void BLERemoteService::removeCharacteristics() {
	for (auto &myPair : m_characteristicMap) {
	   delete myPair.second;
	}
	m_characteristicMap.empty();
} // removeCharacteristics



/**
 * @brief Create a string representation of this remote service.
 * @return A string representation of this remote service.
 */
std::string BLERemoteService::toString() {
	std::ostringstream ss;
	ss << "Service: uuid: " + m_uuid.toString();
	for (auto &myPair : m_characteristicMap) {
		ss << "\n" << myPair.second->toString();
	   // myPair.second is the value
	}
	return ss.str();
} // toString



#endif /* CONFIG_BT_ENABLED */
