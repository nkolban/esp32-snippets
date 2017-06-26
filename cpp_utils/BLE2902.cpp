/*
 * BLE2902.cpp
 *
 *  Created on: Jun 25, 2017
 *      Author: kolban
 */

/*
 * See also:
 * https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
 */
#include "BLE2902.h"

BLE2902::BLE2902() : BLEDescriptor(BLEUUID((uint16_t) 0x2902)) {
	uint8_t data[2] = {0,0};
	setValue(data, 2);
}

BLE2902::~BLE2902() {
}

void BLE2902::setNotifications(bool flag) {
	uint8_t *pValue = getValue();
	if (flag) {
		pValue[0] |= 1<<0;
	} else {
		pValue[0] &= ~(1<<0);
	}
}

void BLE2902::setIndications(bool flag) {
	uint8_t *pValue = getValue();
	if (flag) {
		pValue[0] |= 1<<1;
	} else {
		pValue[0] &= ~(1<<1);
	}
}
