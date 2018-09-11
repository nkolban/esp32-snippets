/*
 * BLEUUID.cpp
 *
 *  Created on: Jun 21, 2017
 *      Author: kolban
 */
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <esp_log.h>
#include <string.h>
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "BLEUUID.h"
static const char* LOG_TAG = "BLEUUID";

#ifdef ARDUINO_ARCH_ESP32
#include "esp32-hal-log.h"
#endif

/**
 * @brief Copy memory from source to target but in reverse order.
 *
 * When we move memory from one location it is normally:
 *
 * ```
 * [0][1][2]...[n] -> [0][1][2]...[n]
 * ```
 *
 * with this function, it is:
 *
 * ```
 * [0][1][2]...[n] -> [n][n-1][n-2]...[0]
 * ```
 *
 * @param [in] target The target of the copy
 * @param [in] source The source of the copy
 * @param [in] size The number of bytes to copy
 */
static void memrcpy(uint8_t* target, uint8_t* source, uint32_t size) {
	assert(size > 0);
	target+=(size-1); // Point target to the last byte of the target data
	while (size > 0) {
		*target = *source;
		target--;
		source++;
		size--;
	}
} // memrcpy


/**
 * @brief Create a UUID from a string.
 *
 * Create a UUID from a string.  There will be two possible stories here.  Either the string represents
 * a binary data field or the string represents a hex encoding of a UUID.
 * For the hex encoding, here is an example:
 *
 * ```
 * "beb5483e-36e1-4688-b7f5-ea07361b26a8"
 *  0 1 2 3  4 5  6 7  8 9  0 1 2 3 4 5
 *  12345678-90ab-cdef-1234-567890abcdef
 * ```
 *
 * This has a length of 36 characters.  We need to parse this into 16 bytes.
 *
 * @param [in] value The string to build a UUID from.
 */
BLEUUID::BLEUUID(std::string value) {
	m_valueSet = true;
	if (value.length() == 2) {
		m_uuid.len         = ESP_UUID_LEN_16;
		m_uuid.uuid.uuid16 = value[0] | (value[1] << 8);
	}
	else if (value.length() == 4) {
		m_uuid.len         = ESP_UUID_LEN_32;
		m_uuid.uuid.uuid32 = value[0] | (value[1] << 8) | (value[2] << 16) | (value[3] << 24);
	}
	else if (value.length() == 16) {
		m_uuid.len = ESP_UUID_LEN_128;
		memrcpy(m_uuid.uuid.uuid128, (uint8_t*)value.data(), 16);
	}
	else if (value.length() == 36) {
// If the length of the string is 36 bytes then we will assume it is a long hex string in
// UUID format.
		m_uuid.len = ESP_UUID_LEN_128;
		int vals[16];
		sscanf(value.c_str(), "%2x%2x%2x%2x-%2x%2x-%2x%2x-%2x%2x-%2x%2x%2x%2x%2x%2x",
			&vals[15],
			&vals[14],
			&vals[13],
			&vals[12],
			&vals[11],
			&vals[10],
			&vals[9],
			&vals[8],
			&vals[7],
			&vals[6],
			&vals[5],
			&vals[4],
			&vals[3],
			&vals[2],
			&vals[1],
			&vals[0]
		);

		int i;
		for (i=0; i<16; i++) {
			m_uuid.uuid.uuid128[i] = vals[i];
		}
	}
	else {
		ESP_LOGE(LOG_TAG, "ERROR: UUID value not 2, 4, 16 or 36 bytes");
		m_valueSet = false;
	}
} //BLEUUID(std::string)


/**
 * @brief Create a UUID from 16 bytes of memory.
 *
 * @param [in] pData The pointer to the start of the UUID.
 * @param [in] size The size of the data.
 * @param [in] msbFirst Is the MSB first in pData memory?
 */
BLEUUID::BLEUUID(uint8_t* pData, size_t size, bool msbFirst) {
	if (size != 16) {
		ESP_LOGE(LOG_TAG, "ERROR: UUID length not 16 bytes");
		return;
	}
	m_uuid.len = ESP_UUID_LEN_128;
	if (msbFirst) {
		memrcpy(m_uuid.uuid.uuid128, pData, 16);
	} else {
		memcpy(m_uuid.uuid.uuid128, pData, 16);
	}
	m_valueSet         = true;
} // BLEUUID


/**
 * @brief Create a UUID from the 16bit value.
 *
 * @param [in] uuid The 16bit short form UUID.
 */
BLEUUID::BLEUUID(uint16_t uuid) {
	m_uuid.len         = ESP_UUID_LEN_16;
	m_uuid.uuid.uuid16 = uuid;
	m_valueSet         = true;

} // BLEUUID


/**
 * @brief Create a UUID from the 32bit value.
 *
 * @param [in] uuid The 32bit short form UUID.
 */
BLEUUID::BLEUUID(uint32_t uuid) {
	m_uuid.len         = ESP_UUID_LEN_32;
	m_uuid.uuid.uuid32 = uuid;
	m_valueSet         = true;
} // BLEUUID


/**
 * @brief Create a UUID from the native UUID.
 *
 * @param [in] uuid The native UUID.
 */
BLEUUID::BLEUUID(esp_bt_uuid_t uuid) {
	m_uuid     = uuid;
	m_valueSet = true;
} // BLEUUID


/**
 * @brief Create a UUID from the ESP32 esp_gat_id_t.
 *
 * @param [in] gattId The data to create the UUID from.
 */
BLEUUID::BLEUUID(esp_gatt_id_t gattId) : BLEUUID(gattId.uuid) {
} // BLEUUID


BLEUUID::BLEUUID() {
	m_valueSet = false;
} // BLEUUID


/**
 * @brief Get the number of bits in this uuid.
 * @return The number of bits in the UUID.  One of 16, 32 or 128.
 */
int BLEUUID::bitSize() {
	if (m_valueSet == false) {
		return 0;
	}
	switch(m_uuid.len) {
		case ESP_UUID_LEN_16: {
			return 16;
		}
		case ESP_UUID_LEN_32: {
			return 32;
		}
		case ESP_UUID_LEN_128: {
			return 128;
		}
		default: {
			ESP_LOGE(LOG_TAG, "Unknown UUID length: %d", m_uuid.len);
			return 0;
		}
	} // End of switch
} // bitSize


/**
 * @brief Compare a UUID against this UUID.
 *
 * @param [in] uuid The UUID to compare against.
 * @return True if the UUIDs are equal and false otherwise.
 */
bool BLEUUID::equals(BLEUUID uuid) {
	//ESP_LOGD(TAG, "Comparing: %s to %s", toString().c_str(), uuid.toString().c_str());
	if (m_valueSet == false || uuid.m_valueSet == false) {
		return false;
	}

	if (uuid.m_uuid.len != m_uuid.len) {
		return uuid.toString() == toString();
	}

	if (uuid.m_uuid.len == ESP_UUID_LEN_16) {
		return uuid.m_uuid.uuid.uuid16 == m_uuid.uuid.uuid16;
	}

	if (uuid.m_uuid.len == ESP_UUID_LEN_32) {
		return uuid.m_uuid.uuid.uuid32 == m_uuid.uuid.uuid32;
	}

	return memcmp(uuid.m_uuid.uuid.uuid128, m_uuid.uuid.uuid128, 16) == 0;
} // equals


/**
 * Create a BLEUUID from a string of the form:
 * 0xNNNN
 * 0xNNNNNNNN
 * 0x<UUID>
 * NNNN
 * NNNNNNNN
 * <UUID>
 */
BLEUUID BLEUUID::fromString(std::string _uuid){
	uint8_t start = 0;
	if (strstr(_uuid.c_str(), "0x") != nullptr) { // If the string starts with 0x, skip those characters.
		start = 2;
	}
	uint8_t len = _uuid.length() - start; // Calculate the length of the string we are going to use.

	if( len == 4) {
		uint16_t x = strtoul(_uuid.substr(start, len).c_str(), NULL, 16);
		return BLEUUID(x);
	} else if (len == 8) {
		uint32_t x = strtoul(_uuid.substr(start, len).c_str(), NULL, 16);
		return BLEUUID(x);
	} else if (len == 36) {
		return BLEUUID(_uuid);
	}
	return BLEUUID();
} // fromString


/**
 * @brief Get the native UUID value.
 *
 * @return The native UUID value or NULL if not set.
 */
esp_bt_uuid_t* BLEUUID::getNative() {
	//ESP_LOGD(TAG, ">> getNative()")
	if (m_valueSet == false) {
		ESP_LOGD(LOG_TAG, "<< Return of un-initialized UUID!");
		return nullptr;
	}
	//ESP_LOGD(TAG, "<< getNative()");
	return &m_uuid;
} // getNative


/**
 * @brief Convert a UUID to its 128 bit representation.
 *
 * A UUID can be internally represented as 16bit, 32bit or the full 128bit.  This method
 * will convert 16 or 32 bit representations to the full 128bit.
 */
BLEUUID BLEUUID::to128() {
	//ESP_LOGD(LOG_TAG, ">> toFull() - %s", toString().c_str());

	// If we either don't have a value or are already a 128 bit UUID, nothing further to do.
	if (m_valueSet == false || m_uuid.len == ESP_UUID_LEN_128) {
		return *this;
	}

	// If we are 16 bit or 32 bit, then set the 4 bytes of the variable part of the UUID.
	if (m_uuid.len == ESP_UUID_LEN_16) {
		uint16_t temp = m_uuid.uuid.uuid16;
		m_uuid.uuid.uuid128[15] = 0;
		m_uuid.uuid.uuid128[14] = 0;
		m_uuid.uuid.uuid128[13] = (temp >> 8) & 0xff;
		m_uuid.uuid.uuid128[12] = temp & 0xff;

	}
	else if (m_uuid.len == ESP_UUID_LEN_32) {
		uint32_t temp = m_uuid.uuid.uuid32;
		m_uuid.uuid.uuid128[15] = (temp >> 24) & 0xff;
		m_uuid.uuid.uuid128[14] = (temp >> 16) & 0xff;
		m_uuid.uuid.uuid128[13] = (temp >> 8) & 0xff;
		m_uuid.uuid.uuid128[12] = temp & 0xff;
	}

	// Set the fixed parts of the UUID.
	m_uuid.uuid.uuid128[11] = 0x00;
	m_uuid.uuid.uuid128[10] = 0x00;

	m_uuid.uuid.uuid128[9]  = 0x10;
	m_uuid.uuid.uuid128[8]  = 0x00;

	m_uuid.uuid.uuid128[7]  = 0x80;
	m_uuid.uuid.uuid128[6]  = 0x00;

	m_uuid.uuid.uuid128[5]  = 0x00;
	m_uuid.uuid.uuid128[4]  = 0x80;
	m_uuid.uuid.uuid128[3]  = 0x5f;
	m_uuid.uuid.uuid128[2]  = 0x9b;
	m_uuid.uuid.uuid128[1]  = 0x34;
	m_uuid.uuid.uuid128[0]  = 0xfb;

	m_uuid.len = ESP_UUID_LEN_128;
	//ESP_LOGD(TAG, "<< toFull <-  %s", toString().c_str());
	return *this;
} // to128




/**
 * @brief Get a string representation of the UUID.
 *
 * The format of a string is:
 * 01234567 8901 2345 6789 012345678901
 * 0000180d-0000-1000-8000-00805f9b34fb
 * 0 1 2 3  4 5  6 7  8 9  0 1 2 3 4 5
 *
 * @return A string representation of the UUID.
 */
std::string BLEUUID::toString() {
	if (m_valueSet == false) {   // If we have no value, nothing to format.
		return "<NULL>";
	}

	// If the UUIDs are 16 or 32 bit, pad correctly.
	std::stringstream ss;

	if (m_uuid.len == ESP_UUID_LEN_16) {  // If the UUID is 16bit, pad correctly.
		ss << "0000" <<
			std::hex <<
			std::setfill('0') <<
			std::setw(4) <<
			m_uuid.uuid.uuid16 <<
			"-0000-1000-8000-00805f9b34fb";
		return ss.str();                    // Return the string
	} // End 16bit UUID

	if (m_uuid.len == ESP_UUID_LEN_32) {  // If the UUID is 32bit, pad correctly.
		ss << std::hex <<
			std::setfill('0') <<
			std::setw(8) <<
			m_uuid.uuid.uuid32 <<
			"-0000-1000-8000-00805f9b34fb";
		return ss.str();                    // return the string
	} // End 32bit UUID

	// The UUID is not 16bit or 32bit which means that it is 128bit.
	//
	// UUID string format:
	// AABBCCDD-EEFF-GGHH-IIJJ-KKLLMMNNOOPP
	//
	ss << std::hex << std::setfill('0') <<
		std::setw(2) << (int)m_uuid.uuid.uuid128[15] <<
		std::setw(2) << (int)m_uuid.uuid.uuid128[14] <<
		std::setw(2) << (int)m_uuid.uuid.uuid128[13] <<
		std::setw(2) << (int)m_uuid.uuid.uuid128[12] << "-" <<
		std::setw(2) << (int)m_uuid.uuid.uuid128[11] <<
		std::setw(2) << (int)m_uuid.uuid.uuid128[10] << "-" <<
		std::setw(2) << (int)m_uuid.uuid.uuid128[9]  <<
		std::setw(2) << (int)m_uuid.uuid.uuid128[8]  << "-" <<
		std::setw(2) << (int)m_uuid.uuid.uuid128[7]  <<
		std::setw(2) << (int)m_uuid.uuid.uuid128[6]  << "-" <<
		std::setw(2) << (int)m_uuid.uuid.uuid128[5]  <<
		std::setw(2) << (int)m_uuid.uuid.uuid128[4]  <<
		std::setw(2) << (int)m_uuid.uuid.uuid128[3]  <<
		std::setw(2) << (int)m_uuid.uuid.uuid128[2]  <<
		std::setw(2) << (int)m_uuid.uuid.uuid128[1]  <<
		std::setw(2) << (int)m_uuid.uuid.uuid128[0];
	return ss.str();
} // toString

#endif /* CONFIG_BT_ENABLED */
