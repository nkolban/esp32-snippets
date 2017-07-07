/*
 * BLEAddress.h
 *
 *  Created on: Jul 2, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLEADDRESS_H_
#define COMPONENTS_CPP_UTILS_BLEADDRESS_H_
#include <esp_gap_ble_api.h> // ESP32 BLE
#include <string>

class BLEAddress {
public:
	BLEAddress(esp_bd_addr_t address);
	BLEAddress(std::string stringAddress);
	virtual ~BLEAddress();
	esp_bd_addr_t *getNative();
	std::string toString();

private:
	esp_bd_addr_t m_address;
};

#endif /* COMPONENTS_CPP_UTILS_BLEADDRESS_H_ */
