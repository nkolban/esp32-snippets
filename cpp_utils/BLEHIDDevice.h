/*
 * BLEHIDDevice.h
 *
 *  Created on: Dec 18, 2017
 *      Author: chegewara
 */

#ifndef _BLEHIDDEVICE_H_
#define _BLEHIDDEVICE_H_
#include "BLECharacteristic.h"
#include "BLEService.h"
#include "BLEDescriptor.h"
#include "BLE2902.h"
#include "HIDTypes.h"

#define GENERIC_HID		960
#define HID_KEYBOARD	961
#define HID_MOUSE		962
#define HID_JOYSTICK	963
#define HID_GAMEPAD		964
#define HID_TABLET		965
#define HID_CARD_READER	966
#define HID_DIGITAL_PEN	967
#define HID_BARCODE		968

class BLEHIDDevice {
public:
	BLEHIDDevice(BLEServer*);
	virtual ~BLEHIDDevice();

	void setReportMap(uint8_t* map, uint16_t);
	void startServices();

	BLEService* deviceInfo();
	BLEService* hidService();
	BLEService* batteryService();

	BLECharacteristic* 	manufacturer();
	BLECharacteristic* 	pnp();
	BLECharacteristic*	hidInfo();
	BLECharacteristic* 	reportMap();
	BLECharacteristic* 	hidControl();
	BLECharacteristic* 	inputReport(void*);
	BLECharacteristic* 	outputReport(void*);
	BLECharacteristic* 	featureReport(void*);
	BLECharacteristic* 	protocolMode();
	BLECharacteristic* 	bootInput();
	BLECharacteristic* 	bootOutput();
	BLECharacteristic* 	batteryLevel(void*);

	BLEDescriptor*		inputReport();
	BLEDescriptor*		outputReport();
	BLEDescriptor*		featureReport();
	BLEDescriptor*		batteryLevel();

private:
	void createCharacteristics();
	void createDescriptors();

	BLEService*			m_deviceInfoService;			//0x180a
	BLEService*			m_hidService;					//0x1812
	BLEService*			m_batteryService = 0;			//0x180f

	BLECharacteristic* 	m_manufacturerCharacteristic;	//0x2a29
	BLECharacteristic* 	m_pnpCharacteristic;			//0x2a50
	BLECharacteristic* 	m_hidInfoCharacteristic;		//0x2a4a
	BLECharacteristic* 	m_reportMapCharacteristic;		//0x2a4b
	BLECharacteristic* 	m_hidControlCharacteristic;		//0x2a4c
	BLECharacteristic* 	m_inputReportCharacteristic;	//0x2a4d
	BLECharacteristic* 	m_outputReportCharacteristic;	//0x2a4d
	BLECharacteristic* 	m_featureReportCharacteristic;	//0x2a4d
	BLECharacteristic* 	m_protocolModeCharacteristic;	//0x2a4e
	BLECharacteristic* 	m_bootInputCharacteristic;		//0x2a22
	BLECharacteristic* 	m_bootOutputCharacteristic;		//0x2a32
	BLECharacteristic*	m_batteryLevelCharacteristic;	//0x2a19

	BLEDescriptor* 		m_inputReportDescriptor;		//0x2908
	BLEDescriptor* 		m_outputReportDescriptor;		//0x2908
	BLEDescriptor* 		m_featureReportDescriptor;		//0x2908
	BLE2902*			m_inputReportNotifications;		//0x2902
	BLE2902*			m_bootInputNotifications;		//0x2902
	BLEDescriptor*		m_batteryLevelDescriptor;		//0x2904
	BLE2902*			m_batteryLevelNotifications;	//0x2902

};

#endif /* _BLEHIDDEVICE_H_ */
