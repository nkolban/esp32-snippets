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

#include "BLEClient.h"
#include "BLERemoteCharacteristic.h"
#include "BLEUUID.h"
#include "FreeRTOS.h"

class BLEClient;
class BLERemoteCharacteristic;


/**
 * @brief A model of a remote %BLE service.
 */
class BLERemoteService {
public:

	virtual ~BLERemoteService();

	// Public methods
	BLERemoteCharacteristic* getCharacteristic(const char* uuid);	
	BLERemoteCharacteristic* getCharacteristic(BLEUUID uuid);
/*<<<<<<< Updated upstream
	BLERemoteCharacteristic* getCharacteristic(uint16_t uuid);
	std::map<std::string, BLERemoteCharacteristic*>* getCharacteristics();
	void getCharacteristics(std::map<uint16_t, BLERemoteCharacteristic*>* ptr);
=======*/
	std::map<BLERemoteCharacteristic*, std::string>* getCharacteristics();
//>>>>>>> Stashed changes

	BLEClient*               getClient(void);
	uint16_t                 getHandle();
	BLEUUID                  getUUID(void);
	std::string              toString(void);

private:
	// Private constructor ... never meant to be created by a user application.
	BLERemoteService(esp_gatt_id_t srvcId, BLEClient* pClient, uint16_t startHandle, uint16_t endHandle);

	// Friends
	friend class BLEClient;
	friend class BLERemoteCharacteristic;

	// Private methods
	void                retrieveCharacteristics(void);
	esp_gatt_id_t*      getSrvcId(void);
	uint16_t            getStartHandle();
	uint16_t            getEndHandle();
	void                gattClientEventHandler(
		esp_gattc_cb_event_t      event,
		esp_gatt_if_t             gattc_if,
		esp_ble_gattc_cb_param_t* evtParam);
	void                removeCharacteristics();

	// Properties

	// We maintain a map of characteristics owned by this service keyed by a string representation of the UUID.

	// We maintain a map of characteristics owned by this service keyed by a handle.
	std::map<uint16_t, BLERemoteCharacteristic *> m_characteristicMapByHandle;
	std::map<BLERemoteCharacteristic *, std::string> m_characteristicMap;

	bool                m_haveCharacteristics; // Have we previously obtained the characteristics.
	BLEClient*          m_pClient;
	FreeRTOS::Semaphore m_semaphoreGetCharEvt = FreeRTOS::Semaphore("GetCharEvt");
	esp_gatt_id_t       m_srvcId;
	BLEUUID             m_uuid;
	uint16_t            m_startHandle;
	uint16_t            m_endHandle;
}; // BLERemoteService

#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLEREMOTESERVICE_H_ */
