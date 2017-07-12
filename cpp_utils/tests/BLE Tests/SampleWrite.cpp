#include "BLE.h"
#include "BLEUtils.h"
#include "BLEServer.h"
#include <esp_log.h>
#include <string>
#include <sys/time.h>
#include <sstream>

#include "sdkconfig.h"

static char LOG_TAG[] = "SampleWrite";

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class MyCallbacks: public BLECharacteristicCallbacks {
	void onWrite(BLECharacteristic *pCharacteristic) {
		if (pCharacteristic->getLength() > 0) {
				ESP_LOGD(LOG_TAG, "*********");
				ESP_LOGD(LOG_TAG, "New value: %.2x", pCharacteristic->getValue()[0]);
				ESP_LOGD(LOG_TAG, "*********");
			}
		}
};

static void run() {
	BLE::initServer("MYDEVICE");
	BLEServer *pServer = new BLEServer();

	BLEService *pService = pServer->createService(BLEUUID(SERVICE_UUID));

	BLECharacteristic *pCharacteristic = pService->createCharacteristic(
		BLEUUID(CHARACTERISTIC_UUID),
		BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
	);

	pCharacteristic->setCallbacks(new MyCallbacks());

	pCharacteristic->setValue("Hello World");

	pService->start();

	BLEAdvertising *pAdvertising = pServer->getAdvertising();
	pAdvertising->start();
}

void SampleWrite(void)
{
	run();
} // app_main
