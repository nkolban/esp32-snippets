#include "BLE.h"
#include "BLEUtils.h"
#include "BLEServer.h"
#include "BLE2902.h"
#include <esp_log.h>
#include <string>
#include <Task.h>

#include "sdkconfig.h"

static char LOG_TAG[] = "SampleServer";

BLECharacteristic* pCharacteristic;

class MyNotifyTask: public Task {
	void run(void *data) {
		while(1) {
			ESP_LOGD(LOG_TAG, "Notify!");
			delay(2000);
			pCharacteristic->indicate(); // Perform the actual indication of a notification to the peer.
		}
	}
};

class MyServer: public BLEServer {
	MyNotifyTask *pMyNotifyTask;
	void onConnect() {
		ESP_LOGD(LOG_TAG, "My onConnect");
		/*
		pMyNotifyTask = new MyNotifyTask();
		pMyNotifyTask->setStackSize(18000);
		pMyNotifyTask->start();
		*/
	}

	void onDisconnect() {
		ESP_LOGD(LOG_TAG, "My onDisconnect");
		pMyNotifyTask->stop();
	}
};

class MainBLEServer: public Task {
	void run(void *data) {
		ESP_LOGD(LOG_TAG, "Starting BLE work!");
		BLE::initServer("MYDEVICE");
		BLEServer* pServer = new MyServer();
		pServer->createApp(0);

		BLEUUID serviceUUID((uint16_t)0x1234);
		BLEService *pService = pServer->createService(serviceUUID);

		pCharacteristic = pService->createCharacteristic(
			BLEUUID((uint16_t)0x99AA),
			BLECharacteristic::PROPERTY_BROADCAST | BLECharacteristic::PROPERTY_READ  |
			BLECharacteristic::PROPERTY_NOTIFY    | BLECharacteristic::PROPERTY_WRITE |
			BLECharacteristic::PROPERTY_INDICATE
		);


		pCharacteristic->setValue("hello steph");

		BLE2902* p2902Descriptor = new BLE2902();
		p2902Descriptor->setNotifications(true);
		pCharacteristic->addDescriptor(p2902Descriptor);

		pService->start();

		//pServer->startAdvertising();
		BLEUUID serviceUUIDFull((uint16_t)0x1234);
		serviceUUIDFull.to128();

		BLEAdvertising* pAdvertising = pServer->getAdvertising();
		pAdvertising->setServiceUUID(serviceUUIDFull);
		pAdvertising->start();
		delay(1000000);
	}
};

void SampleServer(void)
{
	MainBLEServer* pMainBleServer = new MainBLEServer();
	pMainBleServer->setStackSize(20000);
	pMainBleServer->start();

} // app_main
