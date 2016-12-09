#include <esp_partition.h>

const char *partitionTypeToString(esp_partition_type_t type) {
	switch(type) {
	case ESP_PARTITION_TYPE_APP:
		return "APP";
	case ESP_PARTITION_TYPE_DATA:
		return "DATA";
	default:
		return "Unknown";
	}
}
