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
#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEService.h"
#include <string>
#include <unordered_set>

static char tag[] = "BLEDevice";
static const char *adv_type_to_string(uint8_t advType);

extern "C" {
	char *espToString(esp_err_t value);
}

BLEDevice::BLEDevice() {
	m_address = "";
	m_rssi = -9999;
	m_deviceType = 0;
	m_name = "";
	m_appearance = 0;
	m_txPower = 0;
	m_manufacturerType[0] = 0;
	m_manufacturerType[1] = 0;
	m_adFlag = 0;
	m_conn_id = 0;
	m_oncharacteristic = nullptr;
	m_onconnected      = nullptr;
	m_onread           = nullptr;
	m_onsearchcomplete = nullptr;
	m_gattc_if = 0;
	m_haveAdvertizement = false;
}


BLEDevice::BLEDevice(std::string address) : BLEDevice() {
	setAddress(BLEUtils::parseAddress("ff:ff:45:19:14:80"));
}


BLEDevice::~BLEDevice() {
	ESP_LOGD(tag, "BLEDevice object destroyed");
}

/**
 * @brief Given a UUID, retrieve the corresponding service (assuming it exists).
 */
BLEService BLEDevice::findServiceByUUID(esp_bt_uuid_t uuid) {
	assert(uuid.len == ESP_UUID_LEN_16 || uuid.len == ESP_UUID_LEN_32 || uuid.len == ESP_UUID_LEN_128);
	ESP_LOGD(tag, "Looking for service with uuid: %s", BLEUUID(uuid).toString().c_str());
	return m_gattServices.at(uuid);
} // findServiceByUUID

void BLEDevice::readCharacteristic(esp_gatt_srvc_id_t srvcId,
		esp_gatt_id_t characteristicId) {
	esp_err_t errRc = esp_ble_gattc_read_char(m_gattc_if, m_conn_id, &srvcId, &characteristicId, ESP_GATT_AUTH_REQ_NONE);
	if (errRc != ESP_OK) {
		ESP_LOGE(tag, "esp_ble_gattc_read_char: rc=%d %s", errRc, espToString(errRc));
		return;
	}
}

void BLEDevice::readCharacteristic(uint16_t srvcId, uint16_t characteristicId) {
	readCharacteristic(BLEUtils::buildGattSrvcId(BLEUtils::buildGattId(BLEUtils::buildUUID(srvcId))),
			BLEUtils::buildGattId(BLEUtils::buildUUID(characteristicId)));
}


/**
 * @brief Dump the advertizing payload.
 *
 * The payload is a buffer of bytes that is either 31 bytes long or terminated by
 * a 0 length value.  Each entry in the buffer has the format:
 * [length][type][data...]
 *
 * The length does not include itself but does include everything after it until the next record.  A record
 * with a length value of 0 indicates a terminator.
 *
 * https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile
 */
void BLEDevice::setAdvertizementResult(uint8_t *payload) {
	uint8_t length;
	uint8_t ad_type;
	uint8_t sizeConsumed = 0;
	bool finished = false;
	//int i;
	//char text[31*2+1];
	//sprintf(text, "%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x")
	while(!finished) {
		length = *payload;
		payload++;
		sizeConsumed = 1 + length;

		if (length != 0) {
			ad_type = *payload;


			ESP_LOGD(tag, "Type: 0x%.2x (%s), length: %d", ad_type, adv_type_to_string(ad_type), length);
			payload++;
			length--;

			switch(ad_type) {
				case ESP_BLE_AD_TYPE_NAME_CMPL:
					m_name = std::string((char *)payload, length);
					break;
				case ESP_BLE_AD_TYPE_TX_PWR:
					m_txPower = *payload;
					break;
				case ESP_BLE_AD_TYPE_APPEARANCE:
					m_appearance = *(uint16_t *)payload;
					break;
				case ESP_BLE_AD_TYPE_16SRV_PART:
					m_services.insert(std::string((char *) payload, 2));
					break;
				case ESP_BLE_AD_TYPE_128SRV_PART:
					m_services.insert(std::string((char *) payload, 16));
					break;
				case ESP_BLE_AD_TYPE_FLAG:
					setAdFlag((uint8_t)*payload);
					break;
				case ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE:
					assert(length >=2);
					m_manufacturerType[0] = payload[0];
					m_manufacturerType[1] = payload[1];
					break;

				default:
					ESP_LOGD(tag, "Unhandled type");
					break;
			}
			payload += length;
		}


		if (sizeConsumed >=31 || length == 0) {
			finished = true;
		}
	} // !finished
} // dump_adv_payload


/**
 * @brief Open a connection to the %BLE partner.
 */
void BLEDevice::open(esp_gatt_if_t gattc_if) {
	m_gattc_if = gattc_if;
	BLEUtils::registerByAddress(m_address, this);
	esp_err_t errRc = esp_ble_gattc_open(gattc_if, (uint8_t *)m_address.data(), 1);
	if (errRc != ESP_OK) {
		ESP_LOGE(tag, "esp_ble_gattc_open: rc=%d %s", errRc, espToString(errRc));
		return;
	}
} // open


/**
 * @brief Given an advertizing type, return a string representation of the type.
 *
 * For details see ...
 * https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile
 *
 * @return A string representation of the type.
 */
static const char *adv_type_to_string(uint8_t advType) {
	switch(advType) {
	case ESP_BLE_AD_TYPE_FLAG:
		return "ESP_BLE_AD_TYPE_FLAG";
	case ESP_BLE_AD_TYPE_16SRV_PART:
		return "ESP_BLE_AD_TYPE_16SRV_PART";
	case ESP_BLE_AD_TYPE_16SRV_CMPL:
		return "ESP_BLE_AD_TYPE_16SRV_CMPL";
	case ESP_BLE_AD_TYPE_32SRV_PART:
		return "ESP_BLE_AD_TYPE_32SRV_PART";
	case ESP_BLE_AD_TYPE_32SRV_CMPL:
		return "ESP_BLE_AD_TYPE_32SRV_CMPL";
	case ESP_BLE_AD_TYPE_128SRV_PART:
		return "ESP_BLE_AD_TYPE_128SRV_PART";
	case ESP_BLE_AD_TYPE_128SRV_CMPL:
		return "ESP_BLE_AD_TYPE_128SRV_CMPL";
	case ESP_BLE_AD_TYPE_NAME_SHORT:
		return "ESP_BLE_AD_TYPE_NAME_SHORT";
	case ESP_BLE_AD_TYPE_NAME_CMPL:
		return "ESP_BLE_AD_TYPE_NAME_CMPL";
	case ESP_BLE_AD_TYPE_TX_PWR:
		return "ESP_BLE_AD_TYPE_TX_PWR";
	case ESP_BLE_AD_TYPE_DEV_CLASS:
		return "ESP_BLE_AD_TYPE_DEV_CLASS";
	case ESP_BLE_AD_TYPE_SM_TK:
		return "ESP_BLE_AD_TYPE_SM_TK";
	case ESP_BLE_AD_TYPE_SM_OOB_FLAG:
		return "ESP_BLE_AD_TYPE_SM_OOB_FLAG";
	case ESP_BLE_AD_TYPE_INT_RANGE:
		return "ESP_BLE_AD_TYPE_INT_RANGE";
	case ESP_BLE_AD_TYPE_SOL_SRV_UUID:
		return "ESP_BLE_AD_TYPE_SOL_SRV_UUID";
	case ESP_BLE_AD_TYPE_128SOL_SRV_UUID:
		return "ESP_BLE_AD_TYPE_128SOL_SRV_UUID";
	case ESP_BLE_AD_TYPE_SERVICE_DATA:
		return "ESP_BLE_AD_TYPE_SERVICE_DATA";
	case ESP_BLE_AD_TYPE_PUBLIC_TARGET:
		return "ESP_BLE_AD_TYPE_PUBLIC_TARGET";
	case ESP_BLE_AD_TYPE_RANDOM_TARGET:
		return "ESP_BLE_Amap1D_TYPE_RANDOM_TARGET";
	case ESP_BLE_AD_TYPE_APPEARANCE:
		return "ESP_BLE_AD_TYPE_APPEARANCE";
	case ESP_BLE_AD_TYPE_ADV_INT:
		return "ESP_BLE_AD_TYPE_ADV_INT";
	case ESP_BLE_AD_TYPE_32SOL_SRV_UUID:
		return "ESP_BLE_AD_TYPE_32SOL_SRV_UUID";
	case ESP_BLE_AD_TYPE_32SERVICE_DATA:
		return "ESP_BLE_AD_TYPE_32SERVICE_DATA";
	case ESP_BLE_AD_TYPE_128SERVICE_DATA:
		return "ESP_BLE_AD_TYPE_128SERVICE_DATA";
	case ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE:
		return "ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE";
	default:
		ESP_LOGD(tag, "Unknown adv data type: 0x%x", advType);
		return "Unknown";
	}
} // adv_type_to_string


void BLEDevice::addService(esp_gatt_srvc_id_t srvc_id) {
	ESP_LOGD(tag, ">> addService: %s", BLEUUID(srvc_id.id.uuid).toString().c_str());
	//BLEService service;
	//service.setService(srvc_id);
	//m_gattServices.insert(std::pair<esp_bt_uuid_t, BLEService>(srvc_id.id.uuid, service));
} // addService


/**
 * @brief Dump the status of this BLE device.
 */
void BLEDevice::dump() {
	ESP_LOGD(tag, "--- BLEDeviceDump (this=0x%x)", (uint32_t)this);
	if (!m_haveAdvertizement) {
		ESP_LOGD(tag, "No advertizement data");
	} else {
		if (m_address.length() == 6) {
			ESP_LOGD(tag, "address: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x", m_address[0], m_address[1], m_address[2], m_address[3], m_address[4], m_address[5]);
		}
		ESP_LOGD(tag, "rssi: %d", m_rssi);
		if (m_name.length() == 0) {
			ESP_LOGD(tag, "name: <Unknown>");
		} else {
			ESP_LOGD(tag, "name: %s", m_name.c_str());
		}
		ESP_LOGD(tag, "Is limited discoverable?: %d", isLimitedDiscoverable());
		ESP_LOGD(tag, "Is general discoverable?: %d", isGeneralDiscoverable());
		ESP_LOGD(tag, "Is BR/EDR supported?: %d", isBREDRSupported());
		ESP_LOGD(tag, "TX Power: %d", m_txPower);
		ESP_LOGD(tag, "Appearance: %d", m_appearance);
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
 * @brief Set the device address from 6 bytes of storage.
 *
 * @param pData The 6 bytes that correspond to the address.
 */
void BLEDevice::setAddress(ble_address address) {
	m_address = address;
} // setAddress


void BLEDevice::setRSSI(int rssi) {
	m_rssi = rssi;
} // setRSSI


void BLEDevice::setAdFlag(uint8_t adFlag) {
	m_adFlag = adFlag;
} // setAdFlag


void BLEDevice::parsePayload(uint8_t* payload) {
	setAdvertizementResult(payload);
	m_haveAdvertizement = true;
} // parsePayload


/**
 * Retrieve the characteristics for the device service.
 */
void BLEDevice::getCharacteristics(esp_gatt_srvc_id_t *srvc_id, esp_gatt_id_t *lastCharacteristic) {
	ESP_LOGD(tag, ">> BLEDevice::getCharacteristics");
	esp_err_t errRc = esp_ble_gattc_get_characteristic(
		m_gattc_if,
		m_conn_id,
		srvc_id,
		lastCharacteristic // Start characteristic
	);
	if (errRc != ESP_OK) {
		ESP_LOGE(tag, "esp_ble_gattc_get_characteristic: rc=%d %s", errRc, espToString(errRc));
		return;
	}
	ESP_LOGD(tag, "<< BLEDevice::getCharacteristics");
} // getCharacteristics


void BLEDevice::getCharacteristics(BLEService service) {
	// FIX
	/*
	esp_gatt_srvc_id_t tempService = service.getService();
	getCharacteristics(&tempService, nullptr);
	*/
} // getCharacteristics


void BLEDevice::getCharacteristics(BLECharacteristicXXX characteristic) {
	esp_gatt_srvc_id_t srvc_id = characteristic.getSrvcId();
	esp_gatt_id_t lastCharacteristic = characteristic.getCharId();
	getCharacteristics(&srvc_id, &lastCharacteristic);
} // getCharacteristics


void BLEDevice::getDescriptors() {
}


void BLEDevice::searchService() {
	ESP_LOGD(tag, ">> BLEDevice::searchService");
	esp_err_t errRc = esp_ble_gattc_search_service(
		m_gattc_if,
		m_conn_id,
		NULL // Filter UUID
	);
	if (errRc != ESP_OK) {
		ESP_LOGE(tag, "esp_ble_gattc_search_service: rc=%d %s", errRc, espToString(errRc));
		return;
	}
	ESP_LOGD(tag, "<< BLEDevice::searchService");
} // searchService


void BLEDevice::onCharacteristic(BLECharacteristicXXX characteristic) {
	if (m_oncharacteristic != nullptr) {
		m_oncharacteristic(this, characteristic);
	} else {
		ESP_LOGD(tag, "Call to onCharacteristic but no characteristic callback");
	}
} // onCharacteristic


void BLEDevice::onConnected(esp_gatt_status_t status) {
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
void BLEDevice::onRead(std::string data) {
	m_onread(this, data);
} // onRead


/**
 * @brief Indication that a service search has completed.
 *
 * A service search is complete following a call to BLEDevice::searchService() and all the
 * services have been returned from the device.
 */
void BLEDevice::onSearchComplete() {
	if (m_onsearchcomplete != nullptr) {
		m_onsearchcomplete(this);
	} else {
		ESP_LOGD(tag, "Call to onSearchComplete but no serach complete callback");
	}
}

#endif // CONFIG_BT_ENABLED
