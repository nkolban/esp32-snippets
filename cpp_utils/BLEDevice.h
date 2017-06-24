/*
 * BLEDevice.h
 *
 *  Created on: Mar 22, 2017
 *      Author: kolban
 */

#ifndef MAIN_BLEDEVICE_H_
#define MAIN_BLEDEVICE_H_

#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include <esp_gattc_api.h>
#include <string.h>
#include <map>
#include <string>
#include <unordered_set>

#include "BLEService.h"
#include "BLEXXXCharacteristic.h"

typedef std::string ble_address;


/**
 * @brief An operator for comparing items
 */
struct esp_bt_uuid_t_compare {
	bool operator()(const esp_bt_uuid_t &lhs, const esp_bt_uuid_t &rhs) const {
		if (lhs.len != rhs.len) {
			return lhs.len < rhs.len;
		}
		if (lhs.len == ESP_UUID_LEN_16) {
			return lhs.uuid.uuid16 < rhs.uuid.uuid16;
		}
		if (lhs.len == ESP_UUID_LEN_32) {
			return lhs.uuid.uuid32 < rhs.uuid.uuid32;
		}
		if (lhs.len == ESP_UUID_LEN_128) {
			return ::memcmp(lhs.uuid.uuid128, rhs.uuid.uuid128, 16) < 0;
		}
		assert(false); // Shouldn't reach here.
		return true; // Will never reach here.
	} // operator()
}; // esp_bt_uuid_t_compare


/**
 * @brief A %BLE device.
 */
class BLEDevice {
public:
	BLEDevice();
	BLEDevice(std::string address);
	virtual ~BLEDevice();
	void addService(esp_gatt_srvc_id_t srvc_id);
	void dump();

	BLEService findServiceByUUID(esp_bt_uuid_t uuid);
	std::string getAddress() {
		return m_address;
	}
	void getCharacteristics(esp_gatt_srvc_id_t *srvc_id, esp_gatt_id_t *lastCharacteristic);
	void getCharacteristics(BLEService service);
	void getCharacteristics(BLECharacteristicXXX characteristic);
	void getDescriptors();
	bool isBREDRSupported() {
		return (m_adFlag & 0b00100) == 0;
	}
	bool isGeneralDiscoverable() {
		return (m_adFlag & 0b00010) != 0;
	}
	bool isLimitedDiscoverable() {
		return (m_adFlag & 0b00001) != 0;
	}
	void onCharacteristic(BLECharacteristicXXX characteristic);
	void onConnected(esp_gatt_status_t status);
	void onSearchComplete();
	void onRead(std::string data);
	void open(esp_gatt_if_t gattc_if);
	void readCharacteristic(esp_gatt_srvc_id_t srvcId, esp_gatt_id_t characteristicId);
	void readCharacteristic(uint16_t srvcId, uint16_t characteristicId);
	void parsePayload(uint8_t *payload);
	void searchService();
	void setAddress(ble_address address);
	void setAdFlag(uint8_t adFlag);
	void setOnCharacteristic(void (*oncharacteristic)(BLEDevice *pDevice, BLECharacteristicXXX characteristic)) {
		m_oncharacteristic = oncharacteristic;
	}
	/**
	 * @brief Set the function to be called when a connection has been established.
	 */
	void setOnConnected(void (*onconnected)(BLEDevice *pDevice, esp_gatt_status_t status)) {
		m_onconnected = onconnected;
	}

	void setOnRead(	void (*onread)(BLEDevice *pDevice, std::string data)) {
		m_onread = onread;
	}

	void setOnSearchComplete(void (*onsearchcomplete)(BLEDevice *pDevice)) {
		m_onsearchcomplete = onsearchcomplete;
	}
	void setRSSI(int rssi);

private:
	ble_address m_address;
	uint8_t     m_adFlag;
	uint16_t    m_appearance;
	uint16_t    m_conn_id;
	int         m_deviceType;
	esp_gatt_if_t m_gattc_if;
	std::map<esp_bt_uuid_t, BLEService, esp_bt_uuid_t_compare> m_gattServices;
	uint8_t     m_manufacturerType[2];
	std::string m_name;
	void (*m_oncharacteristic)(BLEDevice *pDevice, BLECharacteristicXXX characteristic);
	// The function to be called when a connection has been established.
	void (*m_onconnected)(BLEDevice *pDevice, esp_gatt_status_t status);
	void (*m_onread)(BLEDevice *pDevice, std::string data);
	void (*m_onsearchcomplete)(BLEDevice *pDevice);
	int         m_rssi;
	std::unordered_set<std::string> m_services;
	int8_t      m_txPower;
	void setAdvertizementResult(uint8_t *payload);
	bool m_haveAdvertizement;
}; // class BLEDevice

#endif // CONFIG_BT_ENABLED
#endif /* MAIN_BLEDEVICE_H_ */
