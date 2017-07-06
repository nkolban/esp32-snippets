/*
 * BLEAdvertisedDevice.h
 *
 *  Created on: Jul 3, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLEADVERTISEDDEVICE_H_
#define COMPONENTS_CPP_UTILS_BLEADVERTISEDDEVICE_H_
#include "BLEAddress.h"
#include <esp_gattc_api.h>
#include <map>

class BLEAdvertisedDevice {
public:
	BLEAdvertisedDevice();
	virtual ~BLEAdvertisedDevice();
	std::string getName();
	BLEAddress getAddress();
	int getRSSI();
	std::string toString();
private:
	friend class BLEScan;
	void parseAdvertisement(uint8_t *payload);
	void setAddress(BLEAddress address);
	void setAdFlag(uint8_t adFlag);
	void setAdvertizementResult(uint8_t *payload);
	void setAppearance(uint16_t appearance);
	void setName(std::string name);
	void setRSSI(int rssi);
	void setTXPower(uint8_t txPower);


	BLEAddress  m_address = BLEAddress((uint8_t *)"\0\0\0\0\0\0");
	uint8_t     m_adFlag;
	uint16_t    m_appearance;
	int         m_deviceType;
	uint8_t     m_manufacturerType[2];
	std::string m_name;
	int         m_rssi;
	int8_t      m_txPower;
};

#endif /* COMPONENTS_CPP_UTILS_BLEADVERTISEDDEVICE_H_ */
