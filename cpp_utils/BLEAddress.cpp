/*
 * BLEAddress.cpp
 *
 *  Created on: Jul 2, 2017
 *      Author: kolban
 */

#include "BLEAddress.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <string.h>
#include <stdio.h>

BLEAddress::BLEAddress(esp_bd_addr_t address) {
	memcpy(m_address, address, ESP_BD_ADDR_LEN);
}

BLEAddress::~BLEAddress() {
}

BLEAddress::BLEAddress(std::string stringAddress) {
	if (stringAddress.length() != 17) {
		return;
	}
	int data[6];
	sscanf(stringAddress.c_str(), "%x:%x:%x:%x:%x:%x", &data[0], &data[1], &data[2], &data[3], &data[4], &data[5]);
	m_address[0] = (uint8_t)data[0];
	m_address[1] = (uint8_t)data[1];
	m_address[2] = (uint8_t)data[2];
	m_address[3] = (uint8_t)data[3];
	m_address[4] = (uint8_t)data[4];
	m_address[5] = (uint8_t)data[5];
}

/**
 * @brief Convert a BLE address to a string.
 *
 * @return The string representation of the address.
 */
std::string BLEAddress::toString() {
	std::stringstream stream;
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)((uint8_t *)(m_address))[0] << ':';
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)((uint8_t *)(m_address))[1] << ':';
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)((uint8_t *)(m_address))[2] << ':';
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)((uint8_t *)(m_address))[3] << ':';
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)((uint8_t *)(m_address))[4] << ':';
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)((uint8_t *)(m_address))[5];
	return stream.str();
} // toString

esp_bd_addr_t *BLEAddress::getNative() {
	return &m_address;
}
