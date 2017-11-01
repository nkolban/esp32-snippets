c := cpp_utils
COMPONENT_EXTRA_INCLUDES := $(realpath $c)
COMPONENT_ADD_INCLUDEDIRS := $c
COMPONENT_SRCDIRS := $c

$(call compile_only_if,$(CONFIG_NKOLBAN), \
$c/Task.o \
$c/FreeRTOS.o \
$c/GeneralUtils.o \
)

$(call compile_only_if,$(CONFIG_NKOLBAN_BLE), \
$c/BLEAddress.o \
$c/BLEAdvertisedDevice.o \
$c/BLEAdvertising.o \
$c/BLECharacteristic.o \
$c/BLECharacteristicCallbacks.o \
$c/BLECharacteristicMap.o \
$c/BLEClient.o \
$c/BLEDescriptor.o \
$c/BLEDescriptorMap.o \
$c/BLEDevice.o \
$c/BLERemoteCharacteristic.o \
$c/BLERemoteDescriptor.o \
$c/BLERemoteService.o \
$c/BLEScan.o \
$c/BLEServer.o \
$c/BLEService.o \
$c/BLEServiceMap.o \
$c/BLEUUID.o \
$c/BLEUtils.o \
$c/BLEValue.o \
)

$(call compile_only_if,$(CONFIG_NKOLBAN_BLE2902),$c/BLE2902.o)
