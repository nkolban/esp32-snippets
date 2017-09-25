/*
 * BLERemoteDescriptor.cpp
 *
 *  Created on: Jul 8, 2017
 *      Author: kolban
 */
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include "BLERemoteDescriptor.h"

#endif /* CONFIG_BT_ENABLED */

/**
 * @brief Retrieve the handle associated with this remote descriptor.
 * @return The handle associated with this remote descriptor.
 */
uint16_t BLERemoteDescriptor::getHandle() {
	return m_handle;
} // getHandle


/**
 * @brief Retrieve the UUID associated this remote descriptor.
 * @return The UUID associated this remote descriptor.
 */
BLEUUID BLERemoteDescriptor::getUUID() {
	return m_uuid;
} // getUUID


std::string BLERemoteDescriptor::readValue(void) {
	return "";
} // readValue


uint8_t BLERemoteDescriptor::readUInt8(void) {
	return 0;
} // readUInt8


uint16_t BLERemoteDescriptor::readUInt16(void) {
	return 0;
} // readUInt16


uint32_t BLERemoteDescriptor::readUInt32(void) {
	return 0;
} // readUInt32

std::string BLERemoteDescriptor::toString(void) {
	return "";
} // toString

void BLERemoteDescriptor::writeValue(uint8_t* data, size_t length,
		bool response) {
} // writeValue


void BLERemoteDescriptor::writeValue(std::string newValue, bool response) {
} // writeValue


void BLERemoteDescriptor::writeValue(uint8_t newValue, bool response) {
} // writeValue
