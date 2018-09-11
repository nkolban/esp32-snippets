/*
 * BLEEddystoneURL.cpp
 *
 *  Created on: Mar 12, 2018
 *      Author: pcbreflux
 */
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <string.h>
#include <esp_log.h>
#include "BLEEddystoneURL.h"

static const char LOG_TAG[] = "BLEEddystoneURL";

BLEEddystoneURL::BLEEddystoneURL() {
  beconUUID = 0xFEAA;
  lengthURL = 0;
  m_eddystoneData.frameType = EDDYSTONE_URL_FRAME_TYPE;
  m_eddystoneData.advertisedTxPower = 0;
  memset(m_eddystoneData.url, 0, sizeof(m_eddystoneData.url));
} // BLEEddystoneURL

std::string BLEEddystoneURL::getData() {
	return std::string((char*)&m_eddystoneData, sizeof(m_eddystoneData));
} // getData

BLEUUID BLEEddystoneURL::getUUID() {
	return BLEUUID(beconUUID);
} // getUUID

int8_t BLEEddystoneURL::getPower() {
	return m_eddystoneData.advertisedTxPower;
} // getPower

std::string BLEEddystoneURL::getURL() {
	return std::string((char*)&m_eddystoneData.url, sizeof(m_eddystoneData.url));
} // getURL

std::string BLEEddystoneURL::getDecodedURL() {
	std::string decodedURL = "";

	switch (m_eddystoneData.url[0]) {
		case 0x00:
			decodedURL += "http://www.";
			break;
		case 0x01:
			decodedURL += "https://www.";
			break;
		case 0x02:
			decodedURL += "http://";
			break;
		case 0x03:
			decodedURL += "https://";
			break;
		default:
			decodedURL += m_eddystoneData.url[0];
	}

	for (int i=1;i<lengthURL;i++) {
            if (m_eddystoneData.url[i]>33&&m_eddystoneData.url[i]<127) {
              decodedURL += m_eddystoneData.url[i];
            } else {
              switch (m_eddystoneData.url[i]) {
                case 0x00:
                  decodedURL += ".com/";
                  break;
                case 0x01:
                   decodedURL += ".org/";
                 break;
                case 0x02:
                  decodedURL += ".edu/";
                  break;
                case 0x03:
                  decodedURL += ".net/";
                  break;
                case 0x04:
                  decodedURL += ".info/";
                  break;
                case 0x05:
                  decodedURL += ".biz/";
                  break;
                case 0x06:
                  decodedURL += ".gov/";
                  break;
                case 0x07:
                  decodedURL += ".com";
                  break;
                case 0x08:
                   decodedURL += ".org";
                 break;
                case 0x09:
                  decodedURL += ".edu";
                  break;
                case 0x0A:
                  decodedURL += ".net";
                  break;
                case 0x0B:
                  decodedURL += ".info";
                  break;
                case 0x0C:
                  decodedURL += ".biz";
                  break;
                case 0x0D:
                  decodedURL += ".gov";
                  break;
              }
            }
          }


	return decodedURL;
} // getDecodedURL



/**
 * Set the raw data for the beacon record.
 */
void BLEEddystoneURL::setData(std::string data) {
	if (data.length() > sizeof(m_eddystoneData)) {
		ESP_LOGE(LOG_TAG, "Unable to set the data ... length passed in was %d and max expected %d", data.length(), sizeof(m_eddystoneData));
		return;
	}
  memset(&m_eddystoneData, 0, sizeof(m_eddystoneData));
	memcpy(&m_eddystoneData, data.data(), data.length());
  lengthURL=data.length()-(sizeof(m_eddystoneData)-sizeof(m_eddystoneData.url));

} // setData

void BLEEddystoneURL::setUUID(BLEUUID l_uuid) {
	beconUUID = l_uuid.getNative()->uuid.uuid16;
} // setUUID

void BLEEddystoneURL::setPower(int8_t advertisedTxPower) {
	m_eddystoneData.advertisedTxPower = advertisedTxPower;
} // setPower

void BLEEddystoneURL::setURL(std::string url) {
  if (url.length() > sizeof(m_eddystoneData.url)) {
    ESP_LOGE(LOG_TAG, "Unable to set the url ... length passed in was %d and max expected %d", url.length(), sizeof(m_eddystoneData.url));
    return;
  }
  memset(m_eddystoneData.url, 0, sizeof(m_eddystoneData.url));
  memcpy(m_eddystoneData.url, url.data(), url.length());
  lengthURL=url.length();
} // setURL


#endif
