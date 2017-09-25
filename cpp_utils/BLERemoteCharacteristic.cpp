/*
 * BLERemoteCharacteristic.cpp
 *
 *  Created on: Jul 8, 2017
 *      Author: kolban
 */

#include "BLERemoteCharacteristic.h"

#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include <esp_gattc_api.h>
#include <esp_log.h>
#include <esp_err.h>

#include <sstream>
#include "BLEUtils.h"
#include "GeneralUtils.h"


static const char* LOG_TAG = "BLERemoteCharacteristic";

BLERemoteCharacteristic::BLERemoteCharacteristic(
		uint16_t             handle,
		BLEUUID              uuid,
		esp_gatt_char_prop_t charProp,
		BLERemoteService*    pRemoteService) {
	m_handle         = handle;
	m_uuid           = uuid;
	m_charProp       = charProp;
	m_pRemoteService = pRemoteService;
	m_notifyCallback = nullptr;
} // BLERemoteCharacteristic

/*
static bool compareSrvcId(esp_gatt_srvc_id_t id1, esp_gatt_srvc_id_t id2) {
	if (id1.id.inst_id != id2.id.inst_id) {
		return false;
	}
	if (!BLEUUID(id1.id.uuid).equals(BLEUUID(id2.id.uuid))) {
		return false;
	}
	return true;
} // compareSrvcId
*/

/*
static bool compareGattId(esp_gatt_id_t id1, esp_gatt_id_t id2) {
	if (id1.inst_id != id2.inst_id) {
		return false;
	}
	if (!BLEUUID(id1.uuid).equals(BLEUUID(id2.uuid))) {
		return false;
	}
	return true;
} // compareCharId
*/


/**
 * @brief Handle GATT Client events.
 * When an event arrives for a GATT client we give this characteristic the opportunity to
 * take a look at it to see if there is interest in it.
 * @param [in] event The type of event.
 * @param [in] gattc_if The interface on which the event was received.
 * @param [in] evtParam Payload data for the event.
 * @returns N/A
 */
void BLERemoteCharacteristic::gattClientEventHandler(
	esp_gattc_cb_event_t      event,
	esp_gatt_if_t             gattc_if,
	esp_ble_gattc_cb_param_t* evtParam) {
	switch(event) {
		//
		// ESP_GATTC_READ_CHAR_EVT
		// This event indicates that the server has responded to the read request.
		//
		// read:
		// - esp_gatt_status_t  status
		// - uint16_t           conn_id
		// - uint16_t           handle
		// - uint8_t*           value
		// - uint16_t           value_len
	  //
		case ESP_GATTC_READ_CHAR_EVT: {
			// If this event is not for us, then nothing further to do.
			if (evtParam->read.handle != getHandle()) {
				break;
			}

			// At this point, we have determined that the event is for us, so now we save the value
			// and unlock the semaphore to ensure that the requestor of the data can continue.
			if (evtParam->read.status == ESP_GATT_OK) {
				m_value = std::string((char*)evtParam->read.value, evtParam->read.value_len);
			} else {
				m_value = "";
			}

			m_semaphoreReadCharEvt.give();
			break;
		} // ESP_GATTC_READ_CHAR_EVT


		//
		// ESP_GATTC_REG_FOR_NOTIFY_EVT
		//
		// reg_for_notify:
		// - esp_gatt_status_t status
		// - uint16_t          handle
		//
		case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
			// If the request is not for this BLERemoteCharacteristic then move on to the next.
			if (evtParam->reg_for_notify.handle != getHandle()) {
				break;
			}

			// We have process the notify and can unlock the semaphore.
			m_semaphoreRegForNotifyEvt.give();
			break;
		} // ESP_GATTC_REG_FOR_NOTIFY_EVT


		//
		// ESP_GATTC_WRITE_CHAR_EVT
		//
		// write:
		// - esp_gatt_status_t status
		// - uint16_t          conn_id
		// - uint16_t          handle
		//
		case ESP_GATTC_WRITE_CHAR_EVT: {
			// Determine if this event is for us and, if not, pass onwards.
			if (evtParam->write.handle != getHandle()) {
				break;
			}

			// There is nothing further we need to do here.  This is merely an indication
			// that the write has completed and we can unlock the caller.
			m_semaphoreWriteCharEvt.give();
			break;
		} // ESP_GATTC_WRITE_CHAR_EVT


		default: {
			break;
		}
	} // End switch
}; // gattClientEventHandler

uint16_t BLERemoteCharacteristic::getHandle() {
	ESP_LOGD(LOG_TAG, ">> getHandle: Characteristic: %s", getUUID().toString().c_str());
	ESP_LOGD(LOG_TAG, "<< getHandle: %d 0x%.2x", m_handle, m_handle);
	return m_handle;
}


BLEUUID BLERemoteCharacteristic::getUUID() {
	return m_uuid;
}

/**
 * @brief Read an unsigned 16 bit value
 * @return The unsigned 16 bit value.
 */
uint16_t BLERemoteCharacteristic::readUInt16(void) {
	std::string value = readValue();
	if (value.length() >= 2) {
		return *(uint16_t*)(value.data());
	}
	return 0;
} // readUInt16


/**
 * @brief Read an unsigned 32 bit value.
 * @return the unsigned 32 bit value.
 */
uint32_t BLERemoteCharacteristic::readUInt32(void) {
	std::string value = readValue();
	if (value.length() >= 4) {
		return *(uint32_t*)(value.data());
	}
	return 0;
} // readUInt32


/**
 * @brief Read a byte value
 * @return The value as a byte
 */
uint8_t BLERemoteCharacteristic::readUInt8(void) {
	std::string value = readValue();
	if (value.length() >= 1) {
		return (uint8_t)value[0];
	}
	return 0;
} // readUInt8


/**
 * @brief Read the value of the remote characteristic.
 * @return The value of the remote characteristic.
 */
std::string BLERemoteCharacteristic::readValue() {
	ESP_LOGD(LOG_TAG, ">> readValue(): uuid: %s, handle: %d 0x%.2x", getUUID().toString().c_str(), getHandle(), getHandle());

	m_semaphoreReadCharEvt.take("readValue");

	// Ask the BLE subsystem to retrieve the value for the remote hosted characteristic.
	esp_err_t errRc = ::esp_ble_gattc_read_char(
		m_pRemoteService->getClient()->getGattcIf(),
		m_pRemoteService->getClient()->getConnId(),    // The connection ID to the BLE server
		getHandle(),                                   // The handle of this characteristic
		ESP_GATT_AUTH_REQ_NONE);                       // Security

	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gattc_read_char: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return "";
	}

	// Block waiting for the event that indicates that the read has completed.  When it has, the std::string found
	// in m_value will contain our data.
	m_semaphoreReadCharEvt.wait("readValue");

	ESP_LOGD(LOG_TAG, "<< readValue(): length: %d", m_value.length());
	return m_value;
} // readValue


/**
 * @brief Register for notifications.
 * @param [in] notifyCallback A callback to be invoked for a notification.  If NULL is provided then we are
 * unregistering a notification.
 * @return N/A.
 */
void BLERemoteCharacteristic::registerForNotify(
		void (*notifyCallback)(
			BLERemoteCharacteristic* pBLERemoteCharacteristic,
			uint8_t*                 pData,
			size_t                   length,
			bool                     isNotify)) {
	ESP_LOGD(LOG_TAG, ">> registerForNotify()");

	m_notifyCallback = notifyCallback;   // Save the notification callback.

	m_semaphoreRegForNotifyEvt.take("registerForNotify");

	if (notifyCallback != nullptr) {
		esp_err_t errRc = ::esp_ble_gattc_register_for_notify(
			m_pRemoteService->getClient()->getGattcIf(),
			*m_pRemoteService->getClient()->getPeerAddress().getNative(),
			getHandle()
		);

		if (errRc != ESP_OK) {
			ESP_LOGE(LOG_TAG, "esp_ble_gattc_register_for_notify: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		}
	} // Register
	else {
		esp_err_t errRc = ::esp_ble_gattc_unregister_for_notify(
			m_pRemoteService->getClient()->getGattcIf(),
			*m_pRemoteService->getClient()->getPeerAddress().getNative(),
			getHandle()
		);

		if (errRc != ESP_OK) {
			ESP_LOGE(LOG_TAG, "esp_ble_gattc_unregister_for_notify: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		}
	} // Unregister

	m_semaphoreRegForNotifyEvt.wait("registerForNotify");

	ESP_LOGD(LOG_TAG, "<< registerForNotify()");
} // registerForNotify


/**
 * @brief Convert a BLERemoteCharacteristic to a string representation;
 * @return a String representation.
 */
std::string BLERemoteCharacteristic::toString() {
	std::ostringstream ss;
	ss << "Characteristic: uuid: " << m_uuid.toString() <<
		", handle: " << getHandle() << " 0x" << std::hex << getHandle() <<
		", props: " << BLEUtils::characteristicPropertiesToString(m_charProp);
	return ss.str();
} // toString


/**
 * @brief Write the new value for the characteristic.
 * @param [in] newValue The new value to write.
 * @param [in] response Do we expect a response?
 * @return N/A.
 */
void BLERemoteCharacteristic::writeValue(std::string newValue, bool response) {
	ESP_LOGD(LOG_TAG, ">> writeValue(), length: %d", newValue.length());

	m_semaphoreWriteCharEvt.take("writeValue");

	// Invoke the ESP-IDF API to perform the write.
	esp_err_t errRc = ::esp_ble_gattc_write_char(
		m_pRemoteService->getClient()->getGattcIf(),
		m_pRemoteService->getClient()->getConnId(),
		getHandle(),
		newValue.length(),
		(uint8_t*)newValue.data(),
		response?ESP_GATT_WRITE_TYPE_RSP:ESP_GATT_WRITE_TYPE_NO_RSP,
		ESP_GATT_AUTH_REQ_NONE
	);

	if (errRc != ESP_OK) {
		ESP_LOGE(LOG_TAG, "esp_ble_gattc_write_char: rc=%d %s", errRc, GeneralUtils::errorToString(errRc));
		return;
	}

	m_semaphoreWriteCharEvt.wait("writeValue");

	ESP_LOGD(LOG_TAG, "<< writeValue");
} // writeValue


/**
 * @brief Write the new value for the characteristic.
 *
 * This is a convenience function.  Many BLE characteristics are a single byte of data.
 * @param [in] newValue The new byte value to write.
 * @param [in] response Whether we require a response from the write.
 * @return N/A.
 */
void BLERemoteCharacteristic::writeValue(uint8_t newValue, bool response) {
	writeValue(std::string(reinterpret_cast<char*>(&newValue), 1), response);
} // writeValue


/**
 * @brief Write the new value for the characteristic from a data buffer.
 * @param [in] data A pointer to a data buffer.
 * @param [in] length The length of the data in the data buffer.
 * @param [in] response Whether we require a response from the write.
 */
void BLERemoteCharacteristic::writeValue(uint8_t* data, size_t length, bool response) {
	writeValue(std::string((char *)data, length), response);
} // writeValue


#endif /* CONFIG_BT_ENABLED */
