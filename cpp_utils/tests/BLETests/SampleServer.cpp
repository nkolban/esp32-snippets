/**
 * Create a new BLE server.
 */
#include "BLEUtils.h"
#include "BLEServer.h"
#include "BLE2902.h"
#include <esp_log.h>
#include <string>
#include <Task.h>
#include "BLEDevice.h"

#include "sdkconfig.h"

static char LOG_TAG[] = "SampleServer";

class MainBLEServer: public Task {
	void run(void *data) {
		ESP_LOGD(LOG_TAG, "Starting BLE work!");

		BLEDevice::init("MYDEVICE");
		BLEServer* pServer = new BLEServer();

		BLEService* pService = pServer->createService(BLEUUID((uint16_t)0x1234));

		BLECharacteristic* pCharacteristic = pService->createCharacteristic(
			BLEUUID((uint16_t)0x99AA),
			BLECharacteristic::PROPERTY_BROADCAST | BLECharacteristic::PROPERTY_READ  |
			BLECharacteristic::PROPERTY_NOTIFY    | BLECharacteristic::PROPERTY_WRITE |
			BLECharacteristic::PROPERTY_INDICATE
		);


		pCharacteristic->setValue("Hello World!");

		BLE2902* p2902Descriptor = new BLE2902();
		p2902Descriptor->setNotifications(true);
		pCharacteristic->addDescriptor(p2902Descriptor);

		pService->start();

		BLEAdvertising* pAdvertising = pServer->getAdvertising();
		pAdvertising->setServiceUUID(pService->getUUID().to128());
		pAdvertising->start();

		ESP_LOGD(LOG_TAG, "Advertising started!");
		delay(1000000);
	}
};


void SampleServer(void)
{
	esp_log_level_set("*", ESP_LOG_DEBUG);
	MainBLEServer* pMainBleServer = new MainBLEServer();
	pMainBleServer->setStackSize(20000);
	pMainBleServer->start();

} // app_main
