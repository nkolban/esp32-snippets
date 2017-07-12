#include "BLE.h"
#include "BLEUtils.h"
#include "BLEServer.h"
#include <esp_log.h>
#include <string>

#include "sdkconfig.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

static void run() {
	BLE::initServer("MYDEVICE");

	BLEServer *pServer = new BLEServer();

	BLEService *pService = pServer->createService(BLEUUID(SERVICE_UUID));

	BLECharacteristic *pCharacteristic = pService->createCharacteristic(
		BLEUUID(CHARACTERISTIC_UUID),
		BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
	);

	pCharacteristic->setValue("Hello World says Neil");

	pService->start();

	BLEAdvertising *pAdvertising = pServer->getAdvertising();
	pAdvertising->start();
}

void Sample1(void)
{
	run();
} // app_main
