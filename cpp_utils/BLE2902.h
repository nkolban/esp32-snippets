/*
 * BLE2902.h
 *
 *  Created on: Jun 25, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLE2902_H_
#define COMPONENTS_CPP_UTILS_BLE2902_H_

#include "BLEDescriptor.h"

class BLE2902: public BLEDescriptor {
public:
	BLE2902();
	virtual ~BLE2902();
	void setNotifications(bool flag);
	void setIndications(bool flag);
};

#endif /* COMPONENTS_CPP_UTILS_BLE2902_H_ */
