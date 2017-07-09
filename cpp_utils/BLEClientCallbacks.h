/*
 * BLEClientCallbacks.h
 *
 *  Created on: Jul 5, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLECLIENTCALLBACKS_H_
#define COMPONENTS_CPP_UTILS_BLECLIENTCALLBACKS_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include "BLEClient.h"

class BLEClient;
class BLEClientCallbacks {
public:
	BLEClientCallbacks();
	virtual ~BLEClientCallbacks();
	virtual void onConnect(BLEClient *pClient);
};
#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLECLIENTCALLBACKS_H_ */
