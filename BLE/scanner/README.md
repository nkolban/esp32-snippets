# BLE GAP DUMP 

Code updated to support SDK v2.1

Supports dumping of Bluetooth GAP frames.
For more information about [Bluetooth Generic Access Profile](https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile)

It will dump the header and then start dumping the payload.
Also added is decoding of some payloads frames

In addition to dumping raw frames, the code supports decoding of some frames.
Frames that support decoding:
* 0x16, Subtype 0x1809 Temperature
* 0x16, Subtype 0x180F Battery
* 0x09, Complete local name


|EXAMPLE FRAME DUMP|
|----|
|I (88679) ble1: Device address (bda): ea:ed:37:7a:0f:34|
|I (88689) ble1: Device type         : ESP_BT_DEVICE_TYPE_BLE|
|I (88689) ble1: Search_evt          : ESP_GAP_SEARCH_INQ_RES_EVT|
|I (88689) ble1: Addr_type           : BLE_ADDR_TYPE_RANDOM|
|I (88699) ble1: RSSI                : -85|
|I (88699) ble1: Flag                : 4|
|I (88709) ble1: num_resps           : 1|
|I (88709) ble1: # Payload type: 0x01 (Flags), length: 2|
|I (88719) ble1: * Payload: 04:09 (..)|
|I (88719) ble1: # Payload type: 0x09 (Complete Local Name), length: 9|
|I (88729) ble1: # Complete local name:  43F0DEAE|
|I (88739) ble1: * Payload: 34:33:46:30:44:45:41:45:07 (43F0DEAE.)|
|I (88739) ble1: # Payload type: 0x16 (Service Data - 16-bit UUID), length: 7|
|I (88749) ble1: @ 0x1809 Temperature 28.540000|
|I (88759) ble1: * Payload: 09:18:26:0B:00:FE:04 (..&....)|
|I (88759) ble1: # Payload type: 0x16 (Service Data - 16-bit UUID), length: 4|
|I (88769) ble1: @ 0x180F Battery 100 %|
|I (88769) ble1: * Payload: 0F:18:64:00 (..d.)|
|I (88779) ble1: Payload total length: 26|
|I (88779) ble1:|


I have added prefix to the payload dump to make it easier to read

|Tag| Description |
|---|--- |
|\# | Payload information on the frame level (typical "Frame type" + "Frame length") |
|\* | Raw frame dump (excluding "Frame type")|
|\@ | 16-BIT UUID frames that are decoded (P.t. only 0x1809 and 0x180F is implemented)|
