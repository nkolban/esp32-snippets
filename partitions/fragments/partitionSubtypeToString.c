#include <esp_partition.h>

const char *partitionSubtypeToString(esp_partition_subtype_t subtype) {
	switch(subtype) {
	case ESP_PARTITION_SUBTYPE_APP_FACTORY:
		return "APP_FACTORY";
	case ESP_PARTITION_SUBTYPE_APP_OTA_0:
		return "APP_OTA_0";
	case ESP_PARTITION_SUBTYPE_APP_OTA_1:
		return "APP_OTA_1";
	case ESP_PARTITION_SUBTYPE_APP_OTA_10:
		return "APP_OTA_10";
	case ESP_PARTITION_SUBTYPE_APP_OTA_11:
		return "APP_OTA_11";
	case ESP_PARTITION_SUBTYPE_APP_OTA_12:
		return "APP_OTA_12";
	case ESP_PARTITION_SUBTYPE_APP_OTA_13:
		return "APP_OTA_13";
	case ESP_PARTITION_SUBTYPE_APP_OTA_14:
		return "APP_OTA_14";
	case ESP_PARTITION_SUBTYPE_APP_OTA_15:
		return "APP_OTA_15";
	case ESP_PARTITION_SUBTYPE_APP_OTA_2:
		return "APP_OTA_2";
	case ESP_PARTITION_SUBTYPE_APP_OTA_3:
		return "APP_OTA_3";
	case ESP_PARTITION_SUBTYPE_APP_OTA_4:
		return "APP_OTA_4";
	case ESP_PARTITION_SUBTYPE_APP_OTA_5:
		return "APP_OTA_5";
	case ESP_PARTITION_SUBTYPE_APP_OTA_6:
		return "APP_OTA_6";
	case ESP_PARTITION_SUBTYPE_APP_OTA_7:
		return "APP_OTA_7";
	case ESP_PARTITION_SUBTYPE_APP_OTA_8:
		return "APP_OTA_8";
	case ESP_PARTITION_SUBTYPE_APP_OTA_9:
		return "APP_OTA_9";
	case ESP_PARTITION_SUBTYPE_APP_TEST:
		return "APP_TEST";
	case ESP_PARTITION_SUBTYPE_DATA_ESPHTTPD:
		return "DATA_ESPHTTPD";
	case ESP_PARTITION_SUBTYPE_DATA_FAT:
		return "DATA_FAT";
	case ESP_PARTITION_SUBTYPE_DATA_NVS:
		return "DATA_NVS";
	case ESP_PARTITION_SUBTYPE_DATA_OTA:
		return "DATA_OTA";
	case ESP_PARTITION_SUBTYPE_DATA_PHY:
		return "DATA_PHY";
	case ESP_PARTITION_SUBTYPE_DATA_SPIFFS:
		return "DATA_SPIFFS";
	default:
		return "Unknown";
	}
}
