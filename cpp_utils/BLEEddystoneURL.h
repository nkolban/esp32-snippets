/*
 * BLEEddystoneURL.cpp
 *
 *  Created on: Mar 12, 2018
 *      Author: pcbreflux
 */

#ifndef _BLEEddystoneURL_H_
#define _BLEEddystoneURL_H_
#include "BLEUUID.h"

#define EDDYSTONE_URL_FRAME_TYPE 0x10

/**
 * @brief Representation of a beacon.
 * See:
 * * https://github.com/google/eddystone
 */
class BLEEddystoneURL {
private:
  uint16_t beconUUID;
  uint8_t  lengthURL;
	struct {
		uint8_t frameType;
		int8_t advertisedTxPower;
		uint8_t  url[16];
	} __attribute__((packed))m_eddystoneData;
public:
	BLEEddystoneURL();
	std::string getData();
	BLEUUID     getUUID();
	int8_t      getPower();
	std::string getURL();
	std::string getDecodedURL();
	void        setData(std::string data);
	void        setUUID(BLEUUID l_uuid);
	void        setPower(int8_t advertisedTxPower);
	void        setURL(std::string url);

}; // BLEEddystoneURL

#endif /* _BLEEddystoneURL_H_ */
