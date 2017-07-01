/*
 * BLE.h
 *
 *  Created on: Mar 16, 2017
 *      Author: kolban
 */

#ifndef MAIN_BLE_H_
#define MAIN_BLE_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <esp_gap_ble_api.h> // ESP32 BLE
#include <esp_gattc_api.h>   // ESP32 BLE
#include <map>               // Part of C++ STL
#include <string>

#include "BLEServer.h"
#include "BLEDevice.h"
#include "BLEUtils.h"
/**
 * @brief %BLE functions.
 */
class BLE {
public:
	BLE();
	virtual ~BLE();
	static void dumpDevices();
	static std::map<ble_address, BLEDevice> getDevices();

	static void initClient();
	static void initServer(std::string deviceName);
	static void scan(int duration, esp_ble_scan_type_t scan_type = BLE_SCAN_TYPE_PASSIVE);
	static esp_gatt_if_t getGattcIF();
	static BLEServer *m_bleServer;
}; // class BLE

#endif // CONFIG_BT_ENABLED
#endif /* MAIN_BLE_H_ */
