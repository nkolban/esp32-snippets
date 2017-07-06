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
#include <esp_log.h>
#include <sstream>
#include "BLEAdvertisedDevice.h"
#include "BLEUtils.h"
static const char LOG_TAG[]="BLEAdvertisedDevice";

BLEAdvertisedDevice::BLEAdvertisedDevice() {
	m_rssi                = -9999;
	m_deviceType          = 0;
	m_name                = "";
	m_appearance          = 0;
	m_txPower             = 0;
	m_manufacturerType[0] = 0;
	m_manufacturerType[1] = 0;
	m_adFlag              = 0;
}

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
void BLEAdvertisedDevice::parseAdvertisement(uint8_t *payload) {
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

			ESP_LOGD(LOG_TAG, "Type: 0x%.2x (%s), length: %d", ad_type, BLEUtils::advTypeToString(ad_type), length);

			length--;

			switch(ad_type) {
				case ESP_BLE_AD_TYPE_NAME_CMPL: { // Adv Data Type: 0x09
					setName(std::string((char *)payload, length));
					break;
				} // ESP_BLE_AD_TYPE_NAME_CMPL

				case ESP_BLE_AD_TYPE_TX_PWR: { // Adv Data Type: 0x0A
					setTXPower(*payload);
					break;
				} // ESP_BLE_AD_TYPE_TX_PWR

				case ESP_BLE_AD_TYPE_APPEARANCE: { // Adv Data Type: 0x19
					setAppearance(*(uint16_t *)payload);
					break;
				} // ESP_BLE_AD_TYPE_APPEARANCE

				case ESP_BLE_AD_TYPE_FLAG: { // Adv Data Type: 0x01
					setAdFlag((uint8_t)*payload);
					break;
				} // ESP_BLE_AD_TYPE_FLAG

				case ESP_BLE_AD_TYPE_16SRV_CMPL: { // Adv Data Type: 0x03
					BLEUUID uuid(*(uint16_t *)payload);
					ESP_LOGD(LOG_TAG, "??? %s", uuid.toString().c_str());
					break;
				} // ESP_BLE_AD_TYPE_16SRV_CMPL

				case ESP_BLE_AD_TYPE_16SRV_PART: { // Adv Data Type: 0x02
					BLEUUID uuid(*(uint16_t *)payload);
					ESP_LOGD(LOG_TAG, "??? %s", uuid.toString().c_str());
					break;
				} // ESP_BLE_AD_TYPE_16SRV_PART

				case ESP_BLE_AD_TYPE_32SRV_CMPL: { // Adv Data Type: 0x05
					BLEUUID uuid(*(uint32_t *)payload);
					ESP_LOGD(LOG_TAG, "??? %s", uuid.toString().c_str());
					break;
				} // ESP_BLE_AD_TYPE_32SRV_CMPL

				case ESP_BLE_AD_TYPE_32SRV_PART: { // Adv Data Type: 0x04
					BLEUUID uuid(*(uint32_t *)payload);
					ESP_LOGD(LOG_TAG, "??? %s", uuid.toString().c_str());
					break;
				} // ESP_BLE_AD_TYPE_32SRV_PART

				case ESP_BLE_AD_TYPE_128SRV_CMPL: { // Adv Data Type: 0x07
					BLEUUID uuid(payload, 16);
					ESP_LOGD(LOG_TAG, "??? %s", uuid.toString().c_str());
					break;
				} // ESP_BLE_AD_TYPE_128SRV_CMPL

				case ESP_BLE_AD_TYPE_128SRV_PART: { // Adv Data Type: 0x06
					BLEUUID uuid(payload, 16);
					ESP_LOGD(LOG_TAG, "??? %s", uuid.toString().c_str());
					break;
				} // ESP_BLE_AD_TYPE_128SRV_PART

				/*
				case ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE:
					assert(length >=2);
					//m_manufacturerType[0] = payload[0];
					//m_manufacturerType[1] = payload[1];
					break;
					*/

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


void BLEAdvertisedDevice::setAdFlag(uint8_t adFlag) {
	m_adFlag = adFlag;
} // setAdFlag

void BLEAdvertisedDevice::setAppearance(uint16_t appearance) {
	m_appearance = appearance;
	ESP_LOGD(LOG_TAG, "- appearance: %d", m_appearance);
} // setAppearance

void BLEAdvertisedDevice::setName(std::string name) {
	m_name = name;
	ESP_LOGD(LOG_TAG, "- name: %s", m_name.c_str());
} // setName


void BLEAdvertisedDevice::setRSSI(int rssi) {
	m_rssi = rssi;
	ESP_LOGD(LOG_TAG, "- rssi: %d", m_rssi);
} // setRSSI

void BLEAdvertisedDevice::setTXPower(uint8_t txPower) {
	m_txPower = txPower;
	ESP_LOGD(LOG_TAG, "- txPower: %d", m_txPower);
} // setTXPower


std::string BLEAdvertisedDevice::toString() {
	std::stringstream ss;
	ss << "Name: " << getName() << ", Address: " << getAddress().toString() << ", RSSI: " << getRSSI();
	return ss.str();
} // toString
