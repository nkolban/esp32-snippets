/*
 * BLEServerCallbacks.h
 *
 *  Created on: Jul 4, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLESERVERCALLBACKS_H_
#define COMPONENTS_CPP_UTILS_BLESERVERCALLBACKS_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include "BLEServer.h"
class BLEServer;

class BLEServerCallbacks {
public:
	BLEServerCallbacks();
	virtual ~BLEServerCallbacks();
	virtual void onConnect(BLEServer *pServer);
	virtual void onDisconnect(BLEServer *pServer);
};
#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLESERVERCALLBACKS_H_ */
