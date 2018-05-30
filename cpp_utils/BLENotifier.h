#include "BLERemoteCharacteristic.h"

class BLENotifier {
	public:
		virtual void onNotify(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify)=0;
};