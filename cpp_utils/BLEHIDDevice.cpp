/*
 * BLEHIDDevice.cpp
 *
 *  Created on: Dec 18, 2017
 *      Author: chegewara
 */
//#include "BLEUUID.h"
#include "BLEHIDDevice.h"

BLEHIDDevice::BLEHIDDevice(BLEServer* server) {
	m_deviceInfoService = server->createService(BLEUUID((uint16_t) 0x180a));
	m_hidService = server->createService(BLEUUID((uint16_t) 0x1812), 40);
	//m_batteryService = server->createService(BLEUUID((uint16_t) 0x180f));
	createDescriptors();
	createCharacteristics();
}

BLEHIDDevice::~BLEHIDDevice() {
	// TODO Auto-generated destructor stub
}

void BLEHIDDevice::setReportMap(uint8_t* map, uint16_t size) {
	m_reportMapCharacteristic->setValue(map, size);
}

void BLEHIDDevice::createDescriptors() {
	m_inputReportDescriptor = new BLEDescriptor(BLEUUID((uint16_t)0x2908));
	const uint8_t desc1_val[] = {0x01};
	m_inputReportDescriptor->setValue((uint8_t*)desc1_val, 1);
	m_inputReportNotifications = new BLE2902();

	m_outputReportDescriptor = new BLEDescriptor(BLEUUID((uint16_t)0x2908));
	const uint8_t desc2_val[] = {0x02};
	m_outputReportDescriptor->setValue((uint8_t*)desc2_val, 1);

	m_featureReportDescriptor = new BLEDescriptor(BLEUUID((uint16_t)0x2908));
	const uint8_t desc3_val[] = {0x03};
	m_featureReportDescriptor->setValue((uint8_t*)desc3_val, 1);

	m_bootInputNotifications = new BLE2902();

	if(m_batteryService != nullptr){
		m_batteryLevelDescriptor = new BLEDescriptor(BLEUUID((uint16_t)0x2904));
		m_batteryLevelNotifications = new BLE2902();
	}
}

void BLEHIDDevice::createCharacteristics() {
	m_manufacturerCharacteristic = m_deviceInfoService->createCharacteristic((uint16_t)0x2a29, BLECharacteristic::PROPERTY_READ);
	m_pnpCharacteristic = m_deviceInfoService->createCharacteristic((uint16_t)0x2a50, BLECharacteristic::PROPERTY_READ);

	m_hidInfoCharacteristic = m_hidService->createCharacteristic((uint16_t)0x2a4a, BLECharacteristic::PROPERTY_READ);
	m_reportMapCharacteristic = m_hidService->createCharacteristic((uint16_t)0x2a4b, BLECharacteristic::PROPERTY_READ);
	m_hidControlCharacteristic = m_hidService->createCharacteristic((uint16_t)0x2a4c, BLECharacteristic::PROPERTY_WRITE_NR);
	m_inputReportCharacteristic = m_hidService->createCharacteristic((uint16_t)0x2a4d, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
	m_outputReportCharacteristic = m_hidService->createCharacteristic((uint16_t)0x2a4d, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
	m_featureReportCharacteristic = m_hidService->createCharacteristic((uint16_t)0x2a4d, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
	m_protocolModeCharacteristic = m_hidService->createCharacteristic((uint16_t)0x2a4e, BLECharacteristic::PROPERTY_WRITE_NR);
	m_bootInputCharacteristic = m_hidService->createCharacteristic((uint16_t)0x2a22, BLECharacteristic::PROPERTY_NOTIFY);
	m_bootOutputCharacteristic = m_hidService->createCharacteristic((uint16_t)0x2a32, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);

	m_inputReportCharacteristic->addDescriptor(m_inputReportDescriptor);
	m_inputReportCharacteristic->addDescriptor(m_inputReportNotifications);
	m_outputReportCharacteristic->addDescriptor(m_outputReportDescriptor);
	m_featureReportCharacteristic->addDescriptor(m_featureReportDescriptor);
	m_bootInputCharacteristic->addDescriptor(m_bootInputNotifications);
	if(m_batteryService != nullptr){
		m_batteryLevelCharacteristic->addDescriptor(m_batteryLevelDescriptor);			//OPTIONAL?
		m_batteryLevelCharacteristic->addDescriptor(m_batteryLevelNotifications);		//OPTIONAL?
	}
}

void BLEHIDDevice::startServices() {
	m_deviceInfoService->start();
	m_hidService->start();
	if(m_batteryService!=nullptr)
		m_batteryService->start();
}

BLEService* BLEHIDDevice::deviceInfo() {
	return m_deviceInfoService;
}

BLEService* BLEHIDDevice::hidService() {
	return m_hidService;
}

BLEService* BLEHIDDevice::batteryService() {
	return m_batteryService;
}

BLECharacteristic* 	BLEHIDDevice::manufacturer() {
	return m_manufacturerCharacteristic;
}

BLECharacteristic* 	BLEHIDDevice::pnp() {
	return m_pnpCharacteristic;
}

BLECharacteristic*	BLEHIDDevice::hidInfo() {
	return m_hidInfoCharacteristic;
}

BLECharacteristic* 	BLEHIDDevice::reportMap() {
	return m_reportMapCharacteristic;
}

BLECharacteristic* 	BLEHIDDevice::hidControl() {
	return m_hidControlCharacteristic;
}

BLECharacteristic* 	BLEHIDDevice::inputReport(void*) {
	return m_inputReportCharacteristic;
}

BLECharacteristic* 	BLEHIDDevice::outputReport(void*) {
	return m_outputReportCharacteristic;
}

BLECharacteristic* 	BLEHIDDevice::featureReport(void*) {
	return m_featureReportCharacteristic;
}

BLECharacteristic* 	BLEHIDDevice::protocolMode() {
	return m_protocolModeCharacteristic;
}

BLECharacteristic* 	BLEHIDDevice::bootInput() {
	return m_bootInputCharacteristic;
}

BLECharacteristic* 	BLEHIDDevice::bootOutput() {
	return m_bootOutputCharacteristic;
}

BLECharacteristic* 	BLEHIDDevice::batteryLevel(void*) {
	return m_batteryLevelCharacteristic;
}

BLEDescriptor*		BLEHIDDevice::inputReport() {
	return m_inputReportDescriptor;
}

BLEDescriptor*		BLEHIDDevice::outputReport() {
	return m_outputReportDescriptor;
}

BLEDescriptor*		BLEHIDDevice::featureReport() {
	return m_featureReportDescriptor;
}

BLEDescriptor*		BLEHIDDevice::batteryLevel() {
	return m_batteryLevelDescriptor;
}
