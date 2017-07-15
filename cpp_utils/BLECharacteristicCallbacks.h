/*
 * BLECharacteristicCallbacks.h
 *
 *  Created on: Jul 2, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLECHARACTERISTICCALLBACKS_H_
#define COMPONENTS_CPP_UTILS_BLECHARACTERISTICCALLBACKS_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <BLECharacteristic.h>
class BLECharacteristic;

class BLECharacteristicCallbacks {
public:
	virtual ~BLECharacteristicCallbacks();
	virtual void onRead(BLECharacteristic *pCharacteristic);
	virtual void onWrite(BLECharacteristic *pCharacteristic);
};
#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLECHARACTERISTICCALLBACKS_H_ */
