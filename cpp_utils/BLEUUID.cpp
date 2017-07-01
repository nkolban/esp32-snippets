/*
 * BLEUUID.cpp
 *
 *  Created on: Jun 21, 2017
 *      Author: kolban
 */
#include <esp_log.h>
#include <string.h>
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include "BLEUUID.h"
static char TAG[] = "BLEUUID";


/**
 * @brief Create a UUID from a string.
 * Create a UUID from a string.  There will be two possible stories here.  Either the string represents
 * a binary data field or the string represents a hex encoding of a UUID.
 * For the hex encoding, here is an example:
 * "beb5483e-36e1-4688-b7f5-ea07361b26a8"
 *  0 1 2 3  4 5  6 7  8 9  0 1 2 3 4 5
 * This has a length of 36 characters.  We need to parse this into 16 bytes
 * @param [in] value The string to build a UUID from.
 */
BLEUUID::BLEUUID(std::string value) {
	m_valueSet = true;
	if (value.length() == 2) {
		m_uuid.len = ESP_UUID_LEN_16;
		m_uuid.uuid.uuid16 = value[0] | (value[1] << 8);
	} else if (value.length() == 4) {
		m_uuid.len = ESP_UUID_LEN_32;
		m_uuid.uuid.uuid32 = value[0] | (value[1] << 8) | (value[2] << 16) | (value[3] << 24);
	} else if (value.length() == 16) {
		m_uuid.len = ESP_UUID_LEN_128;
		memcpy(m_uuid.uuid.uuid128, value.data(), 16);
	} else if (value.length() == 36) {
// If the length of the string is 36 bytes then we will assume it is a long hex string in
// UUID format.
		m_uuid.len = ESP_UUID_LEN_128;
		int vals[16];
		sscanf(value.c_str(), "%2x%2x%2x%2x-%2x%2x-%2x%2x-%2x%2x-%2x%2x%2x%2x%2x%2x",
			&vals[0],
			&vals[1],
			&vals[2],
			&vals[3],
			&vals[4],
			&vals[5],
			&vals[6],
			&vals[7],
			&vals[8],
			&vals[9],
			&vals[10],
			&vals[11],
			&vals[12],
			&vals[13],
			&vals[14],
			&vals[15]
		);

		int i;
		for (i=0; i<16; i++) {
			m_uuid.uuid.uuid128[i] = vals[i];
		}
	}
	else {
		ESP_LOGE(TAG, "ERROR: UUID value not 2, 4 or 16 bytes");
		m_valueSet = false;
	}
} //BLEUUID(std::string)


BLEUUID::BLEUUID(uint16_t uuid) {
	m_uuid.len         = ESP_UUID_LEN_16;
	m_uuid.uuid.uuid16 = uuid;
	m_valueSet         = true;
}

BLEUUID::BLEUUID(uint32_t uuid) {
	m_uuid.len         = ESP_UUID_LEN_32;
	m_uuid.uuid.uuid32 = uuid;
	m_valueSet         = true;
}

BLEUUID::BLEUUID(esp_bt_uuid_t uuid) {
	m_uuid     = uuid;
	m_valueSet = true;
}

BLEUUID::BLEUUID() {
	m_valueSet = false;
}

BLEUUID::~BLEUUID() {
	// TODO Auto-generated destructor stub
}


/**
 * @brief Get the native UUID value.
 * @return The native UUID value or NULL if not set.
 */
esp_bt_uuid_t *BLEUUID::getNative() {
	//ESP_LOGD(TAG, ">> getNative()")
	if (m_valueSet == false) {
		ESP_LOGD(TAG, "<< Return of un-initialized UUID!");
		return nullptr;
	}
	//ESP_LOGD(TAG, "<< getNative()");
	return &m_uuid;
}

//01234567 8901 2345 6789 012345678901
//0000180d-0000-1000-8000-00805f9b34fb
//0 1 2 3  4 5  6 7  8 9  0 1 2 3 4 5

/**
 * @brief Get a string representation of the UUID.
 * @return A string representation of the UUID.
 */
std::string BLEUUID::toString() {
	if (m_valueSet == false) {
		return "<NULL>";
	}
	if (m_uuid.len == ESP_UUID_LEN_16) {
		std::stringstream ss;
		ss << "0000" << std::hex << std::setfill('0') << std::setw(4) << m_uuid.uuid.uuid16 << "-0000-1000-8000-00805f9b34fb";
		return ss.str();
	}
	if (m_uuid.len == ESP_UUID_LEN_32) {
		std::stringstream ss;
		ss << std::hex << std::setfill('0') << std::setw(8) << m_uuid.uuid.uuid32 << "-0000-1000-8000-00805f9b34fb";
		return ss.str();
	}
	else {
		std::stringstream ss;
		ss << std::hex << std::setfill('0') <<
				std::setw(2) << (int)m_uuid.uuid.uuid128[0]  <<
				std::setw(2) << (int)m_uuid.uuid.uuid128[1]  <<
				std::setw(2) << (int)m_uuid.uuid.uuid128[2]  <<
				std::setw(2) << (int)m_uuid.uuid.uuid128[3]  << "-" <<
				std::setw(2) << (int)m_uuid.uuid.uuid128[4]  <<
				std::setw(2) << (int)m_uuid.uuid.uuid128[5]  << "-" <<
				std::setw(2) << (int)m_uuid.uuid.uuid128[6]  <<
				std::setw(2) << (int)m_uuid.uuid.uuid128[7]  << "-" <<
				std::setw(2) << (int)m_uuid.uuid.uuid128[8]  <<
				std::setw(2) << (int)m_uuid.uuid.uuid128[9]  << "-" <<
				std::setw(2) << (int)m_uuid.uuid.uuid128[10] <<
				std::setw(2) << (int)m_uuid.uuid.uuid128[11] <<
				std::setw(2) << (int)m_uuid.uuid.uuid128[12] <<
				std::setw(2) << (int)m_uuid.uuid.uuid128[13] <<
				std::setw(2) << (int)m_uuid.uuid.uuid128[14] <<
				std::setw(2) << (int)m_uuid.uuid.uuid128[15];
		return ss.str();
	}
} // toString

/**
 * @brief Compare a UUID against this UUID.
 * @param [in] uuid The UUID to compare against.
 * @return True if the UUIDs are equal and false otherwise.
 */
bool BLEUUID::equals(BLEUUID uuid) {
	//ESP_LOGD(TAG, "Comparing: %s to %s", toString().c_str(), uuid.toString().c_str());
	if (m_valueSet == false) {
		return false;
	}
	if (uuid.m_valueSet == false) {
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
}

void BLEUUID::toFull() {
	//ESP_LOGD(TAG, ">> toFull() - %s", toString().c_str());
	if (m_valueSet == false) {
		return;
	}
	if (m_uuid.len == ESP_UUID_LEN_128) {
		return;
	}
	/*
	if (value.length() == 2) {
		m_uuid.len = ESP_UUID_LEN_16;
		m_uuid.uuid.uuid16 = value[0] | (value[1] << 8);
		m_valueSet = true;
	} else if (value.length() == 4) {
		m_uuid.len = ESP_UUID_LEN_32;
		m_uuid.uuid.uuid32 = value[0] | (value[1] << 8) | (value[2] << 16) | (value[3] << 24);
		m_valueSet = true;
	*/
	if (m_uuid.len == ESP_UUID_LEN_16) {

		uint16_t temp = m_uuid.uuid.uuid16;
		m_uuid.uuid.uuid128[0] = 0;
		m_uuid.uuid.uuid128[1] = 0;
		m_uuid.uuid.uuid128[2] = (temp >> 8) & 0xff;
		m_uuid.uuid.uuid128[3] = temp & 0xff;

	}
	if (m_uuid.len == ESP_UUID_LEN_32) {
		uint32_t temp = m_uuid.uuid.uuid32;
		m_uuid.uuid.uuid128[0] = (temp >> 24) & 0xff;
		m_uuid.uuid.uuid128[1] = (temp >> 16) & 0xff;
		m_uuid.uuid.uuid128[2] = (temp >> 8) & 0xff;
		m_uuid.uuid.uuid128[3] = temp & 0xff;
	}
	m_uuid.len = ESP_UUID_LEN_128;
	m_uuid.uuid.uuid128[4]  = 0x00;
	m_uuid.uuid.uuid128[5]  = 0x00;

	m_uuid.uuid.uuid128[6]  = 0x10;
	m_uuid.uuid.uuid128[7]  = 0x00;

	m_uuid.uuid.uuid128[8]  = 0x80;
	m_uuid.uuid.uuid128[9]  = 0x00;

	m_uuid.uuid.uuid128[10] = 0x00;
	m_uuid.uuid.uuid128[11] = 0x80;
	m_uuid.uuid.uuid128[12] = 0x5f;
	m_uuid.uuid.uuid128[13] = 0x9b;
	m_uuid.uuid.uuid128[14] = 0x34;
	m_uuid.uuid.uuid128[15] = 0xfb;
	//ESP_LOGD(TAG, "<< toFull <-  %s", toString().c_str());
}
