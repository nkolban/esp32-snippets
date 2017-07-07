/*
 * BLEAdvertisedDevice.h
 *
 *  Created on: Jul 3, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLEADVERTISEDDEVICE_H_
#define COMPONENTS_CPP_UTILS_BLEADVERTISEDDEVICE_H_
#include "BLEAddress.h"
#include "BLEScan.h"
#include "BLEUUID.h"
#include <esp_gattc_api.h>
#include <map>

class BLEScan;
class BLEAdvertisedDevice {
public:
	BLEAdvertisedDevice();
	virtual ~BLEAdvertisedDevice();

	BLEAddress  getAddress();
	uint16_t    getApperance();
	std::string getManufacturerData();
	std::string getName();
	int         getRSSI();
	BLEUUID     getServiceUUID();
	BLEScan    *getScan();
	int8_t      getTXPower();
	std::string toString();
	bool        haveAppearance();
	bool        haveManufacturerData();
	bool        haveName();
	bool        haveRSSI();
	bool        haveServiceUUID();
	bool        haveTXPower();

private:
	friend class BLEScan;
	void parseAdvertisement(uint8_t *payload);
	void setAddress(BLEAddress address);
	void setAdFlag(uint8_t adFlag);
	void setAdvertizementResult(uint8_t *payload);
	void setAppearance(uint16_t appearance);
	void setManufacturerData(std::string manufacturerData);
	void setName(std::string name);
	void setRSSI(int rssi);
	void setServiceUUID(BLEUUID serviceUUID);
	void setScan(BLEScan *pScan);
	void setTXPower(int8_t txPower);

	bool m_haveAppearance;
	bool m_haveManufacturerData;
	bool m_haveName;
	bool m_haveRSSI;
	bool m_haveServiceUUID;
	bool m_haveTXPower;


	BLEAddress  m_address = BLEAddress((uint8_t *)"\0\0\0\0\0\0");
	uint8_t     m_adFlag;
	uint16_t    m_appearance;
	int         m_deviceType;
	std::string m_manufacturerData;
	std::string m_name;
	int         m_rssi;
	BLEUUID     m_serviceUUID;
	int8_t      m_txPower;
	BLEScan    *m_pScan;
};

#endif /* COMPONENTS_CPP_UTILS_BLEADVERTISEDDEVICE_H_ */
