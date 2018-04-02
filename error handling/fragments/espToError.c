#include <esp_err.h>
#include <nvs.h>
#include <esp_wifi.h>

char *espToString(esp_err_t value) {
	switch(value) {
	case ESP_OK:
		return "OK";
	case ESP_FAIL:
		return "Fail";
	case ESP_ERR_NO_MEM:
		return "No memory";
	case ESP_ERR_INVALID_ARG:
		return "Invalid argument";
	case ESP_ERR_INVALID_SIZE:
		return "Invalid size";
	case ESP_ERR_INVALID_STATE:
		return "Invalid state";
	case ESP_ERR_NOT_FOUND:
		return "Not found";
	case ESP_ERR_NOT_SUPPORTED:
		return "Not supported";
	case ESP_ERR_TIMEOUT:
		return "Timeout";
	case ESP_ERR_NVS_NOT_INITIALIZED:
		return "ESP_ERR_NVS_NOT_INITIALIZED";
	case ESP_ERR_NVS_NOT_FOUND:
		return "ESP_ERR_NVS_NOT_FOUND";
	case ESP_ERR_NVS_TYPE_MISMATCH:
		return "ESP_ERR_NVS_TYPE_MISMATCH";
	case ESP_ERR_NVS_READ_ONLY:
		return "ESP_ERR_NVS_READ_ONLY";
	case ESP_ERR_NVS_NOT_ENOUGH_SPACE:
		return "ESP_ERR_NVS_NOT_ENOUGH_SPACE";
	case ESP_ERR_NVS_INVALID_NAME:
		return "ESP_ERR_NVS_INVALID_NAME";
	case ESP_ERR_NVS_INVALID_HANDLE:
		return "ESP_ERR_NVS_INVALID_HANDLE";
	case ESP_ERR_NVS_REMOVE_FAILED:
		return "ESP_ERR_NVS_REMOVE_FAILED";
	case ESP_ERR_NVS_KEY_TOO_LONG:
		return "ESP_ERR_NVS_KEY_TOO_LONG";
	case ESP_ERR_NVS_PAGE_FULL:
		return "ESP_ERR_NVS_PAGE_FULL";
	case ESP_ERR_NVS_INVALID_STATE:
		return "ESP_ERR_NVS_INVALID_STATE";
	case ESP_ERR_NVS_INVALID_LENGTH:
		return "ESP_ERR_NVS_INVALID_LENGTH";
	case ESP_ERR_WIFI_NOT_INIT:
		return "ESP_ERR_WIFI_NOT_INIT";
	case ESP_ERR_WIFI_NOT_START:
		return "ESP_ERR_WIFI_NOT_START";
	case ESP_ERR_WIFI_IF:
		return "ESP_ERR_WIFI_IF";
	case ESP_ERR_WIFI_MODE:
		return "ESP_ERR_WIFI_MODE";
	case ESP_ERR_WIFI_STATE:
		return "ESP_ERR_WIFI_STATE";
	case ESP_ERR_WIFI_CONN:
		return "ESP_ERR_WIFI_CONN";
	case ESP_ERR_WIFI_NVS:
		return "ESP_ERR_WIFI_NVS";
	case ESP_ERR_WIFI_MAC:
		return "ESP_ERR_WIFI_MAC";
	case ESP_ERR_WIFI_SSID:
		return "ESP_ERR_WIFI_SSID";
	case ESP_ERR_WIFI_PASSWORD:
		return "ESP_ERR_WIFI_PASSWORD";
	case ESP_ERR_WIFI_TIMEOUT:
		return "ESP_ERR_WIFI_TIMEOUT";
	case ESP_ERR_WIFI_WAKE_FAIL:
		return "ESP_ERR_WIFI_WAKE_FAIL";
	}
	return "Unknown ESP_ERR error";
} // espToString
