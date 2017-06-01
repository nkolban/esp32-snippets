/*
 * GeneralUtils.cpp
 *
 *  Created on: May 20, 2017
 *      Author: kolban
 */

#include "GeneralUtils.h"
#include <esp_log.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <FreeRTOS.h>

static char tag[] = "GeneralUtils";

GeneralUtils::GeneralUtils() {
	// TODO Auto-generated constructor stub

}

GeneralUtils::~GeneralUtils() {
	// TODO Auto-generated destructor stub
}

/*
void GeneralUtils::hexDump(uint8_t* pData, uint32_t length) {
	uint32_t index=0;
	std::stringstream ascii;
	std::stringstream hex;
	char asciiBuf[80];
	char hexBuf[80];
	hex.str("");
	ascii.str("");
	while(index < length) {
		hex << std::setfill('0') << std::setw(2) << std::hex << (int)pData[index] << ' ';
		if (std::isprint(pData[index])) {
			ascii << pData[index];
		} else {
			ascii << '.';
		}
		index++;
		if (index % 16 == 0) {
			strcpy(hexBuf, hex.str().c_str());
			strcpy(asciiBuf, ascii.str().c_str());
			ESP_LOGD(tag, "%s %s", hexBuf, asciiBuf);
			hex.str("");
			ascii.str("");
		}
	}
	if (index %16 != 0) {
		while(index % 16 != 0) {
			hex << "   ";
			index++;
		}
		strcpy(hexBuf, hex.str().c_str());
		strcpy(asciiBuf, ascii.str().c_str());
		ESP_LOGD(tag, "%s %s", hexBuf, asciiBuf);
		//ESP_LOGD(tag, "%s %s", hex.str().c_str(), ascii.str().c_str());
	}
	FreeRTOS::sleep(1000);
}
*/

/*
void GeneralUtils::hexDump(uint8_t* pData, uint32_t length) {
	uint32_t index=0;
	static std::stringstream ascii;
	static std::stringstream hex;
	hex.str("");
	ascii.str("");
	while(index < length) {
		hex << std::setfill('0') << std::setw(2) << std::hex << (int)pData[index] << ' ';
		if (std::isprint(pData[index])) {
			ascii << pData[index];
		} else {
			ascii << '.';
		}
		index++;
		if (index % 16 == 0) {
			ESP_LOGD(tag, "%s %s", hex.str().c_str(), ascii.str().c_str());
			hex.str("");
			ascii.str("");
		}
	}
	if (index %16 != 0) {
		while(index % 16 != 0) {
			hex << "   ";
			index++;
		}
		ESP_LOGD(tag, "%s %s", hex.str().c_str(), ascii.str().c_str());
	}
	FreeRTOS::sleep(1000);
}
*/

/**
 * @brief Dump a representation of binary data to the console.
 *
 * @param [in] pData Pointer to the start of data to be logged.
 * @param [in] length Length of the data (in bytes) to be logged.
 * @return N/A.
 */
void GeneralUtils::hexDump(uint8_t* pData, uint32_t length) {
	char ascii[80];
	char hex[80];
	char tempBuf[80];
	strcpy(ascii, "");
	strcpy(hex, "");
	uint32_t index=0;
	while(index < length) {
		sprintf(tempBuf, "%.2x ", pData[index]);
		strcat(hex, tempBuf);
		if (isprint(pData[index])) {
			sprintf(tempBuf, "%c", pData[index]);
		} else {
			sprintf(tempBuf, ".");
		}
		strcat(ascii, tempBuf);
		index++;
		if (index % 16 == 0) {
			ESP_LOGD(tag, "%s %s", hex, ascii);
			strcpy(ascii, "");
			strcpy(hex, "");
		}
	}
	if (index %16 != 0) {
		while(index % 16 != 0) {
			strcat(hex, "   ");
			index++;
		}
		ESP_LOGD(tag, "%s %s", hex, ascii);
	}
} // hexDump

/**
 * @brief Convert an IP address to string.
 * @param ip The 4 byte IP address.
 * @return A string representation of the IP address.
 */
std::string GeneralUtils::ipToString(uint8_t *ip) {
	std::stringstream s;
	s << (int)ip[0] << '.' << (int)ip[1] << '.' << (int)ip[2] << '.' << (int)ip[3];
	return s.str();
} // ipToString
