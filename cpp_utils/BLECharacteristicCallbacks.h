/*
 * BLECharacteristicCallbacks.h
 *
 *  Created on: Jul 2, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLECHARACTERISTICCALLBACKS_H_
#define COMPONENTS_CPP_UTILS_BLECHARACTERISTICCALLBACKS_H_
#include "BLECharacteristic.h"

class BLECharacteristicCallbacks {
public:
	BLECharacteristicCallbacks();
	virtual ~BLECharacteristicCallbacks();
	virtual void onRead(BLECharacteristic *pCharacteristic);
	virtual void onWrite(BLECharacteristic *pCharacteristic);
};

#endif /* COMPONENTS_CPP_UTILS_BLECHARACTERISTICCALLBACKS_H_ */
