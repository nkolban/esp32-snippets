/*
 * BLEAdvertisedDevice.cpp
 *
 * During the scanning procedure, we will be finding advertised BLE devices.  This class
 * models a found device.
 *
 *
 * See also:
 * https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile
 *
 *  Created on: Jul 3, 2017
 *      Author: kolban
 */
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <esp_log.h>
#include <sstream>
#include "BLEAdvertisedDevice.h"
#include "BLEUtils.h"
static const char LOG_TAG[]="BLEAdvertisedDevice";

BLEAdvertisedDevice::BLEAdvertisedDevice() {
	m_adFlag           = 0;
	m_appearance       = 0;
	m_deviceType       = 0;
	m_manufacturerData = "";
	m_name             = "";
	m_rssi             = -9999;
	m_txPower          = 0;
	m_pScan            = nullptr;

	m_haveAppearance       = false;
	m_haveManufacturerData = false;
	m_haveName             = false;
	m_haveRSSI             = false;
	m_haveServiceUUID      = false;
	m_haveTXPower          = false;

} // BLEAdvertisedDevice


BLEAdvertisedDevice::~BLEAdvertisedDevice() {
}


/**
 * @brief Get the address.
 * @return The address of the advertised device.
 */
BLEAddress BLEAdvertisedDevice::getAddress() {
	return m_address;
} // getAddress


/**
 * @brief Get the appearance.
 * @return The appearance of the advertised device.
 */
uint16_t BLEAdvertisedDevice::getApperance() {
	return m_appearance;
}


/**
 * @brief Get the manufacturer data.
 * @return The manufacturer data of the advertised device.
 */
std::string BLEAdvertisedDevice::getManufacturerData() {
	return m_manufacturerData;
}


/**
 * @brief Get the name.
 * @return The name of the advertised device.
 */
std::string BLEAdvertisedDevice::getName() {
	return m_name;
} // getName


/**
 * @brief Get the RSSI.
 * @return The RSSI of the advertised device.
 */
int BLEAdvertisedDevice::getRSSI() {
	return m_rssi;
} // getRSSI


/**
 * @brief Get the scan object that created this advertisement.
 * @return The scan object.
 */
BLEScan* BLEAdvertisedDevice::getScan() {
	return m_pScan;
} // getScan


/**
 * @brief Get the Service UUID.
 * @return The Service UUID of the advertised device.
 */
BLEUUID BLEAdvertisedDevice::getServiceUUID() {
	return m_serviceUUID;
} // getServiceUUID


/**
 * @brief Get the TX Power.
 * @return The TX Power of the advertised device.
 */
int8_t BLEAdvertisedDevice::getTXPower() {
	return m_txPower;
} // getTXPower


bool BLEAdvertisedDevice::haveAppearance() {
	return m_haveAppearance;
} // haveAppearance


bool BLEAdvertisedDevice::haveManufacturerData() {
	return m_haveManufacturerData;
} // haveManufacturerData


bool BLEAdvertisedDevice::haveName() {
	return m_haveName;
} // haveName


bool BLEAdvertisedDevice::haveRSSI() {
	return m_haveRSSI;
} // haveRSSI


bool BLEAdvertisedDevice::haveServiceUUID() {
	return m_haveServiceUUID;
} // haveServiceUUID


bool BLEAdvertisedDevice::haveTXPower() {
	return m_haveTXPower;
} // haveTXPower


/**
 * @brief Parse the advertising pay load.
 *
 * The pay load is a buffer of bytes that is either 31 bytes long or terminated by
 * a 0 length value.  Each entry in the buffer has the format:
 * [length][type][data...]
 *
 * The length does not include itself but does include everything after it until the next record.  A record
 * with a length value of 0 indicates a terminator.
 *
 * https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile
 */
void BLEAdvertisedDevice::parseAdvertisement(uint8_t* payload) {
	uint8_t length;
	uint8_t ad_type;
	uint8_t sizeConsumed = 0;
	bool finished = false;

	while(!finished) {
		length = *payload; // Retrieve the length of the record.
		payload++; // Skip to type
		sizeConsumed += 1 + length; // increase the size consumed.

		if (length != 0) { // A length of 0 indicate that we have reached the end.
			ad_type = *payload;
			payload++;
			length--;

			char* pHex = BLEUtils::buildHexData(nullptr, payload, length);
			ESP_LOGD(LOG_TAG, "Type: 0x%.2x (%s), length: %d, data: %s",
					ad_type, BLEUtils::advTypeToString(ad_type), length, pHex);
			free(pHex);



			switch(ad_type) {
				case ESP_BLE_AD_TYPE_NAME_CMPL: { // Adv Data Type: 0x09
					setName(std::string(reinterpret_cast<char*>(payload), length));
					break;
				} // ESP_BLE_AD_TYPE_NAME_CMPL

				case ESP_BLE_AD_TYPE_TX_PWR: { // Adv Data Type: 0x0A
					setTXPower(*payload);
					break;
				} // ESP_BLE_AD_TYPE_TX_PWR

				case ESP_BLE_AD_TYPE_APPEARANCE: { // Adv Data Type: 0x19
					setAppearance(*reinterpret_cast<uint16_t*>(payload));
					break;
				} // ESP_BLE_AD_TYPE_APPEARANCE

				case ESP_BLE_AD_TYPE_FLAG: { // Adv Data Type: 0x01
					setAdFlag(*payload);
					break;
				} // ESP_BLE_AD_TYPE_FLAG

				case ESP_BLE_AD_TYPE_16SRV_CMPL: { // Adv Data Type: 0x03
					setServiceUUID(BLEUUID(*reinterpret_cast<uint16_t*>(payload)));
					break;
				} // ESP_BLE_AD_TYPE_16SRV_CMPL

				case ESP_BLE_AD_TYPE_16SRV_PART: { // Adv Data Type: 0x02
					setServiceUUID(BLEUUID(*reinterpret_cast<uint16_t*>(payload)));
					break;
				} // ESP_BLE_AD_TYPE_16SRV_PART

				case ESP_BLE_AD_TYPE_32SRV_CMPL: { // Adv Data Type: 0x05
					setServiceUUID(BLEUUID(*reinterpret_cast<uint32_t*>(payload)));
					break;
				} // ESP_BLE_AD_TYPE_32SRV_CMPL

				case ESP_BLE_AD_TYPE_32SRV_PART: { // Adv Data Type: 0x04
					setServiceUUID(BLEUUID(*reinterpret_cast<uint32_t*>(payload)));
					break;
				} // ESP_BLE_AD_TYPE_32SRV_PART

				case ESP_BLE_AD_TYPE_128SRV_CMPL: { // Adv Data Type: 0x07
					setServiceUUID(BLEUUID(payload, 16, false));
					break;
				} // ESP_BLE_AD_TYPE_128SRV_CMPL

				case ESP_BLE_AD_TYPE_128SRV_PART: { // Adv Data Type: 0x06
					setServiceUUID(BLEUUID(payload, 16, false));
					break;
				} // ESP_BLE_AD_TYPE_128SRV_PART

				// See CSS Part A 1.4 Manufacturer Specific Data
				case ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE: {
					setManufacturerData(std::string(reinterpret_cast<char*>(payload), length));
					break;
				} // ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE

				default: {
					ESP_LOGD(LOG_TAG, "Unhandled type");
					break;
				}
			} // switch
			payload += length;
		} // Length <> 0


		if (sizeConsumed >=31 || length == 0) {
			finished = true;
		}
	} // !finished
} // parseAdvertisement


/**
 * @brief Set the address of the advertised device.
 * @param [in] address The address of the advertised device.
 */
void BLEAdvertisedDevice::setAddress(BLEAddress address) {
	m_address = address;
} // setAddress


/**
 * @brief Set the adFlag for this device.
 * @param [in] The discovered adFlag.
 */
void BLEAdvertisedDevice::setAdFlag(uint8_t adFlag) {
	m_adFlag = adFlag;
} // setAdFlag


/**
 * @brief Set the appearance for this device.
 * @param [in] The discovered appearance.
 */
void BLEAdvertisedDevice::setAppearance(uint16_t appearance) {
	m_appearance     = appearance;
	m_haveAppearance = true;
	ESP_LOGD(LOG_TAG, "- appearance: %d", m_appearance);
} // setAppearance


/**
 * @brief Set the manufacturer data for this device.
 * @param [in] The discovered manufacturer data.
 */
void BLEAdvertisedDevice::setManufacturerData(std::string manufacturerData) {
	m_manufacturerData     = manufacturerData;
	m_haveManufacturerData = true;
	char* pHex = BLEUtils::buildHexData(nullptr, (uint8_t*)m_manufacturerData.data(), (uint8_t)m_manufacturerData.length());
	ESP_LOGD(LOG_TAG, "- manufacturer data: %s", pHex);
	free(pHex);
} // setManufacturerData


/**
 * @brief Set the name for this device.
 * @param [in] name The discovered name.
 */
void BLEAdvertisedDevice::setName(std::string name) {
	m_name     = name;
	m_haveName = true;
	ESP_LOGD(LOG_TAG, "- name: %s", m_name.c_str());
} // setName


/**
 * @brief Set the RSSI for this device.
 * @param [in] rssi The discovered RSSI.
 */
void BLEAdvertisedDevice::setRSSI(int rssi) {
	m_rssi     = rssi;
	m_haveRSSI = true;
	ESP_LOGD(LOG_TAG, "- rssi: %d", m_rssi);
} // setRSSI


/**
 * @brief Set the Scan that created this advertised device.
 * @param pScan The Scan that created this advertised device.
 */
void BLEAdvertisedDevice::setScan(BLEScan* pScan) {
	m_pScan = pScan;
} // setScan

/**
 * @brief Set the Service UUID for this device.
 * @param [in] serviceUUID The discovered serviceUUID
 */
void BLEAdvertisedDevice::setServiceUUID(BLEUUID serviceUUID) {
	m_serviceUUID     = serviceUUID;
	m_haveServiceUUID = true;
	ESP_LOGD(LOG_TAG, "- serviceUUID: %s", serviceUUID.toString().c_str());
} // setRSSI


/**
 * @brief Set the power level for this device.
 * @param [in] txPower The discovered power level.
 */
void BLEAdvertisedDevice::setTXPower(int8_t txPower) {
	m_txPower     = txPower;
	m_haveTXPower = true;
	ESP_LOGD(LOG_TAG, "- txPower: %d", m_txPower);
} // setTXPower


/**
 * @brief Create a string representation of this device.
 * @return A string representation of this device.
 */
std::string BLEAdvertisedDevice::toString() {
	std::stringstream ss;
	ss << "Name: " << getName() << ", Address: " << getAddress().toString();
	if (haveAppearance()) {
		ss << ", appearance: " << getApperance();
	}
	if (haveManufacturerData()) {
		char *pHex = BLEUtils::buildHexData(nullptr, (uint8_t*)getManufacturerData().data(), getManufacturerData().length());
		ss << ", manufacturer data: " << pHex;
		free(pHex);
	}
	if (haveServiceUUID()) {
		ss << ", serviceUUID: " << getServiceUUID().toString();
	}
	if (haveTXPower()) {
		ss << ", txPower: " << (int)getTXPower();
	}
	return ss.str();
} // toString

#endif /* CONFIG_BT_ENABLED */

