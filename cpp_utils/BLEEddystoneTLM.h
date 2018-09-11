/*
 * BLEEddystoneTLM.cpp
 *
 *  Created on: Mar 12, 2018
 *      Author: pcbreflux
 */

#ifndef _BLEEddystoneTLM_H_
#define _BLEEddystoneTLM_H_
#include "BLEUUID.h"

#define EDDYSTONE_TLM_FRAME_TYPE 0x20

/**
 * @brief Representation of a beacon.
 * See:
 * * https://github.com/google/eddystone
 */
class BLEEddystoneTLM {
private:
  uint16_t beconUUID;
	struct {
		uint8_t frameType;
		int8_t version;
		uint16_t volt;
		uint16_t temp;
		uint32_t advCount;
		uint32_t tmil;
	} __attribute__((packed))m_eddystoneData;
public:
	BLEEddystoneTLM();
	std::string getData();
	BLEUUID     getUUID();
	uint8_t     getVersion();
	uint16_t    getVolt();
	float       getTemp();
	uint32_t    getCount();
	uint32_t    getTime();
	std::string toString();
	void        setData(std::string data);
	void        setUUID(BLEUUID l_uuid);
	void        setVersion(uint8_t version);
	void        setVolt(uint16_t volt);
	void        setTemp(float temp);
	void        setCount(uint32_t advCount);
	void        setTime(uint32_t tmil);

}; // BLEEddystoneTLM

#endif /* _BLEEddystoneTLM_H_ */
