/*
 * BLERemoteService.h
 *
 *  Created on: Jul 8, 2017
 *      Author: kolban
 */

#ifndef COMPONENTS_CPP_UTILS_BLEREMOTESERVICE_H_
#define COMPONENTS_CPP_UTILS_BLEREMOTESERVICE_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include <map>
#include "BLERemoteCharacteristic.h"
#include "BLEUUID.h"

class BLERemoteService {
public:
	BLERemoteService(BLEUUID uuid);
	virtual ~BLERemoteService();
	void getCharacteristics();
private:
	std::map<std::string, BLERemoteCharacteristic *> m_characteristicMap;
	BLEUUID m_uuid;
};

#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLEREMOTESERVICE_H_ */
