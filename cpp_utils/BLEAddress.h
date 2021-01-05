/*
 * BLEAddress.h
 *
 *  Created on: Jul 2, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLEADDRESS_H_
#define COMPONENTS_CPP_UTILS_BLEADDRESS_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <esp_gap_ble_api.h> // ESP32 BLE
#include <string>


/**
 * @brief A %BLE device address.
 *
 * Every %BLE device has a unique address which can be used to identify it and form connections.
 */
class BLEAddress {
public:
	BLEAddress(esp_bd_addr_t address, esp_ble_wl_addr_type_t type = BLE_WL_ADDR_TYPE_RANDOM);
	BLEAddress(std::string stringAddress, esp_ble_wl_addr_type_t type = BLE_WL_ADDR_TYPE_RANDOM);
	bool           equals(BLEAddress otherAddress);
	esp_bd_addr_t* getNative();
	std::string    toString();
    esp_ble_wl_addr_type_t getType() const;

private:
	esp_bd_addr_t m_address;
    esp_ble_wl_addr_type_t m_type;
};

#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLEADDRESS_H_ */
