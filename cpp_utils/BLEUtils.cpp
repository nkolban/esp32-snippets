/*
 * BLEUtils.cpp
 *
 *  Created on: Mar 25, 2017
 *      Author: kolban
 */
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)
#include "BLEUtils.h"
#include "BLEUUID.h"
#include "BLEDevice.h"


#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <bt.h>              // ESP32 BLE
#include <esp_bt_main.h>     // ESP32 BLE
#include <esp_gap_ble_api.h> // ESP32 BLE
#include <esp_gattc_api.h>   // ESP32 BLE
#include <esp_err.h>         // ESP32 ESP-IDF
#include <esp_log.h>         // ESP32 ESP-IDF
#include <map>               // Part of C++ STL
#include <sstream>
#include <iomanip>

static char LOG_TAG[] = "BLEUtils";

static std::map<ble_address, BLEDevice *> g_addressMap;
static std::map<uint16_t, BLEDevice *> g_connIdMap;

BLEUtils::BLEUtils() {
}

BLEUtils::~BLEUtils() {
}


std::string bt_utils_gatt_close_reason_to_string(esp_gatt_conn_reason_t reason) {
	switch(reason) {
		case ESP_GATT_CONN_UNKNOWN:
			return "ESP_GATT_CONN_UNKNOWN";
		case ESP_GATT_CONN_L2C_FAILURE:
			return "ESP_GATT_CONN_L2C_FAILURE";
		case ESP_GATT_CONN_TIMEOUT:
			return "ESP_GATT_CONN_TIMEOUT";
		case ESP_GATT_CONN_TERMINATE_PEER_USER:
			return "ESP_GATT_CONN_TERMINATE_PEER_USER";
		case ESP_GATT_CONN_TERMINATE_LOCAL_HOST:
			return "ESP_GATT_CONN_TERMINATE_LOCAL_HOST";
		case ESP_GATT_CONN_FAIL_ESTABLISH:
			return "ESP_GATT_CONN_FAIL_ESTABLISH";
		case ESP_GATT_CONN_LMP_TIMEOUT:
			return "ESP_GATT_CONN_LMP_TIMEOUT";
		case ESP_GATT_CONN_CONN_CANCEL:
			return "ESP_GATT_CONN_CONN_CANCEL";
		case ESP_GATT_CONN_NONE:
			return "ESP_GATT_CONN_NONE";
		default:
			return "Unknown";
	}
} // bt_utils_gatt_close_reason_to_string


/**
 * @brief Convert a BT GAP event type to a string representation.
 */

std::string gapEventToString(uint32_t eventType) {
	switch(eventType) {
		case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
			return "ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT";
		case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
			return "ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT";
		case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
			return "ESP_GAP_BLE_ADV_START_COMPLETE_EVT";
		case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
			return "ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT";
		case ESP_GAP_BLE_SCAN_RESULT_EVT:
			return "ESP_GAP_BLE_SCAN_RESULT_EVT";
		case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
			return "ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT";
		case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
			return "ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT";
		case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
			return "ESP_GAP_BLE_SCAN_START_COMPLETE_EVT";
		case ESP_GAP_BLE_AUTH_CMPL_EVT:                              /* Authentication complete indication. */
			return "ESP_GAP_BLE_AUTH_CMPL_EVT";
		case ESP_GAP_BLE_KEY_EVT:                                    /* BLE  key event for peer device keys */
			return "ESP_GAP_BLE_KEY_EVT";
		case ESP_GAP_BLE_SEC_REQ_EVT:                                /* BLE  security request */
			return "ESP_GAP_BLE_SEC_REQ_EVT";
		case ESP_GAP_BLE_PASSKEY_NOTIF_EVT:                          /* passkey notification event */
			return "ESP_GAP_BLE_PASSKEY_NOTIF_EVT";
		case ESP_GAP_BLE_PASSKEY_REQ_EVT:                            /* passkey request event */
			return "ESP_GAP_BLE_PASSKEY_REQ_EVT";
		case ESP_GAP_BLE_OOB_REQ_EVT:                                /* OOB request event */
			return "ESP_GAP_BLE_OOB_REQ_EVT";
		case ESP_GAP_BLE_LOCAL_IR_EVT:                               /* BLE local IR event */
			return "ESP_GAP_BLE_LOCAL_IR_EVT";
		case ESP_GAP_BLE_LOCAL_ER_EVT:                               /* BLE local ER event */
			return "ESP_GAP_BLE_LOCAL_ER_EVT";
		case ESP_GAP_BLE_NC_REQ_EVT:                                 /* Numeric Comparison request event */
			return "ESP_GAP_BLE_NC_REQ_EVT";
		case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:                      /*!< When stop adv complete, the event comes */
			return "ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT";
		case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
			return "ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT";
		default:
			ESP_LOGD(LOG_TAG, "gapEventToString: Unknown event type 0x%x", eventType);
			return "Unknown event type";
	}
} // gapEventToString


std::string bt_utils_gatt_client_event_type_to_string(esp_gattc_cb_event_t eventType) {
	switch(eventType) {
		case ESP_GATTC_ACL_EVT:
			return "ESP_GATTC_ACL_EVT";
		case ESP_GATTC_ADV_DATA_EVT:
			return "ESP_GATTC_ADV_DATA_EVT";
		case ESP_GATTC_ADV_VSC_EVT:
			return "ESP_GATTC_ADV_VSC_EVT";
		case ESP_GATTC_BTH_SCAN_CFG_EVT:
			return "ESP_GATTC_BTH_SCAN_CFG_EVT";
		case ESP_GATTC_BTH_SCAN_DIS_EVT:
			return "ESP_GATTC_BTH_SCAN_DIS_EVT";
		case ESP_GATTC_BTH_SCAN_ENB_EVT:
			return "ESP_GATTC_BTH_SCAN_ENB_EVT";
		case ESP_GATTC_BTH_SCAN_PARAM_EVT:
			return "ESP_GATTC_BTH_SCAN_PARAM_EVT";
		case ESP_GATTC_BTH_SCAN_RD_EVT:
			return "ESP_GATTC_BTH_SCAN_RD_EVT";
		case ESP_GATTC_BTH_SCAN_THR_EVT:
			return "ESP_GATTC_BTH_SCAN_THR_EVT";
		case ESP_GATTC_CANCEL_OPEN_EVT:
			return "ESP_GATTC_CANCEL_OPEN_EVT";
		case ESP_GATTC_CFG_MTU_EVT:
			return "ESP_GATTC_CFG_MTU_EVT";
		case ESP_GATTC_CLOSE_EVT:
			return "ESP_GATTC_CLOSE_EVT";
		case ESP_GATTC_CONGEST_EVT:
			return "ESP_GATTC_CONGEST_EVT";
		case ESP_GATTC_ENC_CMPL_CB_EVT:
			return "ESP_GATTC_ENC_CMPL_CB_EVT";
		case ESP_GATTC_EXEC_EVT:
			return "ESP_GATTC_EXEC_EVT";
		case ESP_GATTC_GET_CHAR_EVT:
			return "ESP_GATTC_GET_CHAR_EVT";
		case ESP_GATTC_GET_DESCR_EVT:
			return "ESP_GATTC_GET_DESCR_EVT";
		case ESP_GATTC_GET_INCL_SRVC_EVT:
			return "ESP_GATTC_GET_INCL_SRVC_EVT";
		case ESP_GATTC_MULT_ADV_DATA_EVT:
			return "ESP_GATTC_MULT_ADV_DATA_EVT";
		case ESP_GATTC_MULT_ADV_DIS_EVT:
			return "ESP_GATTC_MULT_ADV_DIS_EVT";
		case ESP_GATTC_MULT_ADV_ENB_EVT:
			return "ESP_GATTC_MULT_ADV_ENB_EVT";
		case ESP_GATTC_MULT_ADV_UPD_EVT:
			return "ESP_GATTC_MULT_ADV_UPD_EVT";
		case ESP_GATTC_NOTIFY_EVT:
			return "ESP_GATTC_NOTIFY_EVT";
		case ESP_GATTC_OPEN_EVT:
			return "ESP_GATTC_OPEN_EVT";
		case ESP_GATTC_PREP_WRITE_EVT:
			return "ESP_GATTC_PREP_WRITE_EVT";
		case ESP_GATTC_READ_CHAR_EVT:
			return "ESP_GATTC_READ_CHAR_EVT";
		case ESP_GATTC_REG_EVT:
			return "ESP_GATTC_REG_EVT";
		case ESP_GATTC_REG_FOR_NOTIFY_EVT:
			return "ESP_GATTC_REG_FOR_NOTIFY_EVT";
		case ESP_GATTC_SCAN_FLT_CFG_EVT:
			return "ESP_GATTC_SCAN_FLT_CFG_EVT";
		case ESP_GATTC_SCAN_FLT_PARAM_EVT:
			return "ESP_GATTC_SCAN_FLT_PARAM_EVT";
		case ESP_GATTC_SCAN_FLT_STATUS_EVT:
			return "ESP_GATTC_SCAN_FLT_STATUS_EVT";
		case ESP_GATTC_SEARCH_CMPL_EVT:
			return "ESP_GATTC_SEARCH_CMPL_EVT";
		case ESP_GATTC_SEARCH_RES_EVT:
			return "ESP_GATTC_SEARCH_RES_EVT";
		case ESP_GATTC_SRVC_CHG_EVT:
			return "ESP_GATTC_SRVC_CHG_EVT";
		case ESP_GATTC_READ_DESCR_EVT:
			return "ESP_GATTC_READ_DESCR_EVT";
		case ESP_GATTC_UNREG_EVT:
			return "ESP_GATTC_UNREG_EVT";
		case ESP_GATTC_UNREG_FOR_NOTIFY_EVT:
			return "ESP_GATTC_UNREG_FOR_NOTIFY_EVT";
		case ESP_GATTC_WRITE_CHAR_EVT:
			return "ESP_GATTC_WRITE_CHAR_EVT";
		case ESP_GATTC_WRITE_DESCR_EVT:
			return "ESP_GATTC_WRITE_DESCR_EVT";
		default:
			return "Unknown";
	}
} // bt_utils_gatt_event_type_to_string


/**
 * @brief Return a string representation of a GATT server event code.
 * @param [in] eventType A GATT server event code.
 * @return A string representation of the GATT server event code.
 */
std::string bt_utils_gatt_server_event_type_to_string(esp_gatts_cb_event_t eventType) {
	switch(eventType) {
	case ESP_GATTS_REG_EVT:
		return "ESP_GATTS_REG_EVT";
	case ESP_GATTS_READ_EVT:
		return "ESP_GATTS_READ_EVT";
	case ESP_GATTS_WRITE_EVT:
		return "ESP_GATTS_WRITE_EVT";
	case ESP_GATTS_EXEC_WRITE_EVT:
		return "ESP_GATTS_EXEC_WRITE_EVT";
	case ESP_GATTS_MTU_EVT:
		return "ESP_GATTS_MTU_EVT";
	case ESP_GATTS_CONF_EVT:
		return "ESP_GATTS_CONF_EVT";
	case ESP_GATTS_UNREG_EVT:
		return "ESP_GATTS_UNREG_EVT";
	case ESP_GATTS_CREATE_EVT:
		return "ESP_GATTS_CREATE_EVT";
	case ESP_GATTS_ADD_INCL_SRVC_EVT:
		return "ESP_GATTS_ADD_INCL_SRVC_EVT";
	case ESP_GATTS_ADD_CHAR_EVT:
		return "ESP_GATTS_ADD_CHAR_EVT";
	case ESP_GATTS_ADD_CHAR_DESCR_EVT:
		return "ESP_GATTS_ADD_CHAR_DESCR_EVT";
	case ESP_GATTS_DELETE_EVT:
		return "ESP_GATTS_DELETE_EVT";
	case ESP_GATTS_START_EVT:
		return "ESP_GATTS_START_EVT";
	case ESP_GATTS_STOP_EVT:
		return "ESP_GATTS_STOP_EVT";
	case ESP_GATTS_CONNECT_EVT:
		return "ESP_GATTS_CONNECT_EVT";
	case ESP_GATTS_DISCONNECT_EVT:
		return "ESP_GATTS_DISCONNECT_EVT";
	case ESP_GATTS_OPEN_EVT:
		return "ESP_GATTS_OPEN_EVT";
	case ESP_GATTS_CANCEL_OPEN_EVT:
		return "ESP_GATTS_CANCEL_OPEN_EVT";
	case ESP_GATTS_CLOSE_EVT:
		return "ESP_GATTS_CLOSE_EVT";
	case ESP_GATTS_LISTEN_EVT:
		return "ESP_GATTS_LISTEN_EVT";
	case ESP_GATTS_CONGEST_EVT:
		return "ESP_GATTS_CONGEST_EVT";
	case ESP_GATTS_RESPONSE_EVT:
		return "ESP_GATTS_RESPONSE_EVT";
	case ESP_GATTS_CREAT_ATTR_TAB_EVT:
		return "ESP_GATTS_CREAT_ATTR_TAB_EVT";
	case ESP_GATTS_SET_ATTR_VAL_EVT:
		return "ESP_GATTS_SET_ATTR_VAL_EVT";
	}
	return "Unknown";
}


/**
 * @brief Convert an esp_ble_addr_type_t to a string representation.
 */
/*
static std::string bt_addr_t_to_string(esp_ble_addr_type_t type) {
	switch(type) {
		case BLE_ADDR_TYPE_PUBLIC:
			return "BLE_ADDR_TYPE_PUBLIC";
		case BLE_ADDR_TYPE_RANDOM:
			return "BLE_ADDR_TYPE_RANDOM";
		case BLE_ADDR_TYPE_RPA_PUBLIC:
			return "BLE_ADDR_TYPE_RPA_PUBLIC";
		case BLE_ADDR_TYPE_RPA_RANDOM:
			return "BLE_ADDR_TYPE_RPA_RANDOM";
		default:
			return "Unknown addr_t";
	}
} // bt_addr_t_to_string
*/

/**
 * @brief Convert a BLE device type to a string.
 */
std::string BLEUtils::devTypeToString(esp_bt_dev_type_t type) {
	switch(type) {
	case ESP_BT_DEVICE_TYPE_BREDR:
		return "ESP_BT_DEVICE_TYPE_BREDR";
	case ESP_BT_DEVICE_TYPE_BLE:
		return "ESP_BT_DEVICE_TYPE_BLE";
	case ESP_BT_DEVICE_TYPE_DUMO:
		return "ESP_BT_DEVICE_TYPE_DUMO";
	default:
		return "Unknown";
	}
} // bt_dev_type_to_string



/**
 * @brief Convert an esp_gatt_id_t to a string.
 */
static std::string gattIdToString(esp_gatt_id_t gattId) {
	std::stringstream stream;
	stream << "uuid: " << BLEUUID(gattId.uuid).toString() << ", inst_id: " << (int)gattId.inst_id;
	//sprintf(buffer, "uuid: %s, inst_id: %d", uuidToString(gattId.uuid).c_str(), gattId.inst_id);
	return stream.str();
} // gattIdToString


/**
 * @brief Create a hex representation of data.
 *
 * @param [in] target Where to write the hex string.  If this is null, we malloc storage.
 * @param [in] source The start of the binary data.
 * @param [in] length The length of the data to convert.
 * @return A pointer to the formatted buffer.
 */
char *BLEUtils::buildHexData(uint8_t *target, uint8_t *source, uint8_t length) {
// Guard against too much data.
	if (length > 100) {
		length = 100;
	}


	if (target == nullptr) {
		target = (uint8_t *)malloc(length * 2 + 1);
		if (target == nullptr) {
			ESP_LOGE(LOG_TAG, "buildHexData: malloc failed");
			return nullptr;
		}
	}
	char *startOfData = (char *)target;

	int i;
	for (i=0; i<length; i++) {
		sprintf((char *)target, "%.2x", (char)*source);
		source++;
		target +=2;
	}

// Handle the special case where there was no data.
	if (length == 0) {
		*startOfData = 0;
	}

	return startOfData;
} // buildHexData


typedef struct {
	uint32_t assignedNumber;
	std::string name;
} characteristicMap_t;

static characteristicMap_t g_characteristicsMappings[] = {
		{0x2a00, "Device Name"},
		{0x2a01, "Appearance"},
		{0, ""}
};

/**
 * @brief Mapping from service ids to names
 */
typedef struct {
	std::string name;
	std::string type;
	uint32_t assignedNumber;
} gattService_t;


/**
 * Definition of the service ids to names that we know about.
 */
static gattService_t g_gattServices[] = {
		{"Alert Notification Service", "org.bluetooth.service.alert_notification", 0x1811},
		{"Automation IO", "org.bluetooth.service.automation_io",	0x1815 },
		{"Battery Service","org.bluetooth.service.battery_service",	0x180F},
		{"Blood Pressure", "org.bluetooth.service.blood_pressure", 0x1810},
		{"Body Composition", "org.bluetooth.service.body_composition", 0x181B},
		{"Bond Management", "org.bluetooth.service.bond_management", 0x181E},
		{"Continuous Glucose Monitoring", "org.bluetooth.service.continuous_glucose_monitoring", 0x181F},
		{"Current Time Service", "org.bluetooth.service.current_time", 0x1805},
		{"Cycling Power", "org.bluetooth.service.cycling_power", 0x1818},
		{"Cycling Speed and Cadence", "org.bluetooth.service.cycling_speed_and_cadence", 0x1816},
		{"Device Information", "org.bluetooth.service.device_information", 0x180A},
		{"Environmental Sensing", "org.bluetooth.service.environmental_sensing", 0x181A},
		{"Generic Access", "org.bluetooth.service.generic_access", 0x1800},
		{"Generic Attribute", "org.bluetooth.service.generic_attribute", 0x1801},
		{"Glucose", "org.bluetooth.service.glucose", 0x1808},
		{"Health Thermometer", "org.bluetooth.service.health_thermometer", 0x1809},
		{"Heart Rate", "org.bluetooth.service.heart_rate", 0x180D},
		{"HTTP Proxy", "org.bluetooth.service.http_proxy", 0x1823},
		{"Human Interface Device", "org.bluetooth.service.human_interface_device", 0x1812},
		{"Immediate Alert", "org.bluetooth.service.immediate_alert", 0x1802},
		{"Indoor Positioning", "org.bluetooth.service.indoor_positioning", 0x1821},
		{"Internet Protocol Support", "org.bluetooth.service.internet_protocol_support", 0x1820},
		{"Link Loss", "org.bluetooth.service.link_loss", 0x1803},
		{"Location and Navigation", "org.bluetooth.service.location_and_navigation", 0x1819},
		{"Next DST Change Service", "org.bluetooth.service.next_dst_change", 0x1807},
		{"Object Transfer", "org.bluetooth.service.object_transfer", 0x1825},
		{"Phone Alert Status Service", "org.bluetooth.service.phone_alert_status", 0x180E},
		{"Pulse Oximeter", "org.bluetooth.service.pulse_oximeter", 0x1822},
		{"Reference Time Update Service", "org.bluetooth.service.reference_time_update", 0x1806},
		{"Running Speed and Cadence", "org.bluetooth.service.running_speed_and_cadence", 0x1814},
		{"Scan Parameters", "org.bluetooth.service.scan_parameters", 0x1813},
		{"Transport Discovery", "org.bluetooth.service.transport_discovery", 0x1824},
		{"Tx Power", "org.bluetooth.service.tx_power", 0x1804},
		{"User Data", "org.bluetooth.service.user_data", 0x181C},
		{"Weight Scale", "org.bluetooth.service.weight_scale", 0x181D},
		{"", "", 0 }
};


std::string BLEUtils::gattServiceToString(uint32_t serviceId) {
	gattService_t *p = g_gattServices;
	while (p->name.length() > 0) {
		if (p->assignedNumber == serviceId) {
			return p->name;
		}
		p++;
	}
	return "Unknown";
} // gattServiceToString


std::string BLEUtils::gattCharacteristicUUIDToString(uint32_t characteristicUUID) {
	characteristicMap_t *p = g_characteristicsMappings;
	while (p->name.length() > 0) {
		if (p->assignedNumber == characteristicUUID) {
			return p->name;
		}
		p++;
	}
	return "Unknown";
}

/**
 * @brief Convert a GATT status to a string.
 *
 * @param [in] status The status to convert.
 * @return A string representation of the status.
 */
std::string BLEUtils::gattStatusToString(esp_gatt_status_t status) {
	switch(status) {
		case ESP_GATT_OK:
			return "ESP_GATT_OK";
		case ESP_GATT_INVALID_HANDLE:
			return "ESP_GATT_INVALID_HANDLE";
		case ESP_GATT_READ_NOT_PERMIT:
			return "ESP_GATT_READ_NOT_PERMIT";
		case ESP_GATT_WRITE_NOT_PERMIT:
			return "ESP_GATT_WRITE_NOT_PERMIT";
		case ESP_GATT_INVALID_PDU:
			return "ESP_GATT_INVALID_PDU";
		case ESP_GATT_INSUF_AUTHENTICATION:
			return "ESP_GATT_INSUF_AUTHENTICATION";
		case ESP_GATT_REQ_NOT_SUPPORTED:
			return "ESP_GATT_REQ_NOT_SUPPORTED";
		case ESP_GATT_INVALID_OFFSET:
			return "ESP_GATT_INVALID_OFFSET";
		case ESP_GATT_INSUF_AUTHORIZATION:
			return "ESP_GATT_INSUF_AUTHORIZATION";
		case ESP_GATT_PREPARE_Q_FULL:
			return "ESP_GATT_PREPARE_Q_FULL";
		case ESP_GATT_NOT_FOUND:
			return "ESP_GATT_NOT_FOUND";
		case ESP_GATT_NOT_LONG:
			return "ESP_GATT_NOT_LONG";
		case ESP_GATT_INSUF_KEY_SIZE:
			return "ESP_GATT_INSUF_KEY_SIZE";
		case ESP_GATT_INVALID_ATTR_LEN:
			return "ESP_GATT_INVALID_ATTR_LEN";
		case ESP_GATT_ERR_UNLIKELY:
			return "ESP_GATT_ERR_UNLIKELY";
		case ESP_GATT_INSUF_ENCRYPTION:
			return "ESP_GATT_INSUF_ENCRYPTION";
		case ESP_GATT_UNSUPPORT_GRP_TYPE:
			return "ESP_GATT_UNSUPPORT_GRP_TYPE";
		case ESP_GATT_INSUF_RESOURCE:
			return "ESP_GATT_INSUF_RESOURCE";
		case ESP_GATT_NO_RESOURCES:
			return "ESP_GATT_NO_RESOURCES";
		case ESP_GATT_INTERNAL_ERROR:
			return "ESP_GATT_INTERNAL_ERROR";
		case ESP_GATT_WRONG_STATE:
			return "ESP_GATT_WRONG_STATE";
		case ESP_GATT_DB_FULL:
			return "ESP_GATT_DB_FULL";
		case ESP_GATT_BUSY:
			return "ESP_GATT_BUSY";
		case ESP_GATT_ERROR:
			return "ESP_GATT_ERROR";
		case ESP_GATT_CMD_STARTED:
			return "ESP_GATT_CMD_STARTED";
		case ESP_GATT_ILLEGAL_PARAMETER:
			return "ESP_GATT_ILLEGAL_PARAMETER";
		case ESP_GATT_PENDING:
			return "ESP_GATT_PENDING";
		case ESP_GATT_AUTH_FAIL:
			return "ESP_GATT_AUTH_FAIL";
		case ESP_GATT_MORE:
			return "ESP_GATT_MORE";
		case ESP_GATT_INVALID_CFG:
			return "ESP_GATT_INVALID_CFG";
		case ESP_GATT_SERVICE_STARTED:
			return "ESP_GATT_SERVICE_STARTED";
		case ESP_GATT_ENCRYPED_NO_MITM:
			return "ESP_GATT_ENCRYPED_NO_MITM";
		case ESP_GATT_NOT_ENCRYPTED:
			return "ESP_GATT_NOT_ENCRYPTED";
		case ESP_GATT_CONGESTED:
			return "ESP_GATT_CONGESTED";
		case ESP_GATT_DUP_REG:
			return "ESP_GATT_DUP_REG";
		case ESP_GATT_ALREADY_OPEN:
			return "ESP_GATT_ALREADY_OPEN";
		case ESP_GATT_CANCEL:
			return "ESP_GATT_CANCEL";
		case ESP_GATT_CCC_CFG_ERR:
			return "ESP_GATT_CCC_CFG_ERR";
		case ESP_GATT_PRC_IN_PROGRESS:
			return "ESP_GATT_PRC_IN_PROGRESS";
		default:
			return "Unknown";
	}
} // bt_utils_gatt_status_to_string


static std::string characteristic_properties_to_string(esp_gatt_char_prop_t prop) {
	std::stringstream stream;
	stream <<
			"broadcast: "  << ((prop & ESP_GATT_CHAR_PROP_BIT_BROADCAST)?"1":"0") <<
			", read: "     << ((prop & ESP_GATT_CHAR_PROP_BIT_READ)?"1":"0") <<
			", write_nr: " << ((prop & ESP_GATT_CHAR_PROP_BIT_WRITE_NR)?"1":"0") <<
			", write: "    << ((prop & ESP_GATT_CHAR_PROP_BIT_WRITE)?"1":"0") <<
			", notify: "   << ((prop & ESP_GATT_CHAR_PROP_BIT_NOTIFY)?"1":"0") <<
			", indicate: " << ((prop & ESP_GATT_CHAR_PROP_BIT_INDICATE)?"1":"0") <<
			", auth: "     << ((prop & ESP_GATT_CHAR_PROP_BIT_AUTH)?"1":"0");
	return stream.str();
} // characteristic_properties_to_string


/**
 * @brief convert a GAP search event to a string.
 */
std::string bt_gap_search_event_type_to_string(uint32_t searchEvt) {
	switch(searchEvt) {
		case ESP_GAP_SEARCH_INQ_RES_EVT:
			return "ESP_GAP_SEARCH_INQ_RES_EVT";
		case ESP_GAP_SEARCH_INQ_CMPL_EVT:
			return "ESP_GAP_SEARCH_INQ_CMPL_EVT";
		case ESP_GAP_SEARCH_DISC_RES_EVT:
			return "ESP_GAP_SEARCH_DISC_RES_EVT";
		case ESP_GAP_SEARCH_DISC_BLE_RES_EVT:
			return "ESP_GAP_SEARCH_DISC_BLE_RES_EVT";
		case ESP_GAP_SEARCH_DISC_CMPL_EVT:
			return "ESP_GAP_SEARCH_DISC_CMPL_EVT";
		case ESP_GAP_SEARCH_DI_DISC_CMPL_EVT:
			return "ESP_GAP_SEARCH_DI_DISC_CMPL_EVT";
		case ESP_GAP_SEARCH_SEARCH_CANCEL_CMPL_EVT:
			return "ESP_GAP_SEARCH_SEARCH_CANCEL_CMPL_EVT";
		default:
			ESP_LOGD(LOG_TAG, "Unknown event type: 0x%x", searchEvt);
			return "Unknown event type";
	}
} // bt_gap_search_event_type_to_string

esp_gatt_id_t BLEUtils::buildGattId(esp_bt_uuid_t uuid, uint8_t inst_id) {
	esp_gatt_id_t retGattId;
	retGattId.uuid = uuid;
	retGattId.inst_id = inst_id;
	return retGattId;
}

esp_gatt_srvc_id_t BLEUtils::buildGattSrvcId(esp_gatt_id_t gattId,
		bool is_primary) {
	esp_gatt_srvc_id_t retSrvcId;
	retSrvcId.id = gattId;
	retSrvcId.is_primary = is_primary;
	return retSrvcId;
}

/**
 * @brief Decode and dump a GATT client event
 *
 * @param [in] event The type of event received.
 * @param [in] evtParam The data associated with the event.
 */
void BLEUtils::dumpGattClientEvent(
	esp_gattc_cb_event_t event,
	esp_gatt_if_t gattc_if,
	esp_ble_gattc_cb_param_t *evtParam) {

	//esp_ble_gattc_cb_param_t *evtParam = (esp_ble_gattc_cb_param_t *)param;
	ESP_LOGD(LOG_TAG, "GATT Event: %s", bt_utils_gatt_client_event_type_to_string(event).c_str());
	switch(event) {
		//
		// ESP_GATTC_CLOSE_EVT
		//
		case ESP_GATTC_CLOSE_EVT: {
			ESP_LOGD(LOG_TAG, "status: %s, reason:%s, conn_id: %d",
				BLEUtils::gattStatusToString(evtParam->close.status).c_str(),
				bt_utils_gatt_close_reason_to_string(evtParam->close.reason).c_str(),
				evtParam->close.conn_id);
			break;
		}

		//
		// ESP_GATTC_GET_CHAR_EVT
		//
		case ESP_GATTC_GET_CHAR_EVT: {
			std::string description = "Unknown";
			if (evtParam->get_char.char_id.uuid.len == ESP_UUID_LEN_16) {
				description = BLEUtils::gattCharacteristicUUIDToString(evtParam->get_char.char_id.uuid.uuid.uuid16);
			}
			ESP_LOGD(LOG_TAG, "[status: %s, conn_id: %d, srvc_id: %s, char_id: %s [description: %s]\nchar_prop: %s]",
					BLEUtils::gattStatusToString(evtParam->get_char.status).c_str(),
				evtParam->get_char.conn_id,
				BLEUtils::gattServiceIdToString(evtParam->get_char.srvc_id).c_str(),
				gattIdToString(evtParam->get_char.char_id).c_str(),
				description.c_str(),
				characteristic_properties_to_string(evtParam->get_char.char_prop).c_str()
			);
			break;
		}

		//
		// ESP_GATTC_OPEN_EVT
		//
		case ESP_GATTC_OPEN_EVT: {
			ESP_LOGD(LOG_TAG, "status: %s", BLEUtils::gattStatusToString(evtParam->open.status).c_str());
			ESP_LOGD(LOG_TAG, "conn_id: %d", evtParam->open.conn_id);
			ESP_LOGD(LOG_TAG, "device address: %s", BLEUtils::addressToString(std::string((char *)evtParam->open.remote_bda, 6)).c_str());
			ESP_LOGD(LOG_TAG, "MTU: %d", evtParam->open.mtu);
			break;
		} // ESP_GATTC_OPEN_EVT


		//
		// ESP_GATTC_READ_CHAR_EVT
		//
		case ESP_GATTC_READ_CHAR_EVT: {
			ESP_LOGD(LOG_TAG, "[status: %s, conn_id: %d, srvc_id: <%s>, char_id: <%s>, descr_id: <%s>, value_type: 0x%x, value_len: %d]",
				BLEUtils::gattStatusToString(evtParam->read.status).c_str(),
				evtParam->read.conn_id,
				BLEUtils::gattServiceIdToString(evtParam->read.srvc_id).c_str(),
				gattIdToString(evtParam->read.char_id).c_str(),
				gattIdToString(evtParam->read.descr_id).c_str(),
				evtParam->read.value_type,
				evtParam->read.value_len
			);
			if (evtParam->read.status == ESP_GATT_OK) {
				char *pHexData = BLEUtils::buildHexData(nullptr, evtParam->read.value, evtParam->read.value_len);
				ESP_LOGD(LOG_TAG, "value: %s", pHexData);
				free(pHexData);
			}
			break;
		} // ESP_GATTC_READ_CHAR_EVT

		//
		// ESP_GATTC_REG_EVT
		//
		case ESP_GATTC_REG_EVT: {
			ESP_LOGD(LOG_TAG, "status: %s, client_if: 0x%x, app_id: 0x%x",
					BLEUtils::gattStatusToString(evtParam->reg.status).c_str(),
				//evtParam->reg.gatt_if,
				gattc_if,
				evtParam->reg.app_id);
			break;
		}

		//
		// ESP_GATTC_SEARCH_CMP_EVT
		//
		case ESP_GATTC_SEARCH_CMPL_EVT: {
			ESP_LOGD(LOG_TAG, "status: %s, conn_id: %d",
					BLEUtils::gattStatusToString(evtParam->search_cmpl.status).c_str(),
				evtParam->search_cmpl.conn_id);
			break;
		}

		//
		// ESP_GATTC_SEARCH_RES_EVT
		//
		case ESP_GATTC_SEARCH_RES_EVT: {
			std::string name = "";
			if (evtParam->search_res.srvc_id.id.uuid.len == ESP_UUID_LEN_16) {
				name = BLEUtils::gattServiceToString(evtParam->search_res.srvc_id.id.uuid.uuid.uuid16);
			}
			if (name.length() == 0) {
				name = "??";
			}

			ESP_LOGD(LOG_TAG, "srvc_id: %s [%s], instanceId: 0x%.2x conn_id: %d",
				BLEUtils::gattServiceIdToString(evtParam->search_res.srvc_id).c_str(),
				name.c_str(),
				evtParam->search_res.srvc_id.id.inst_id,
				evtParam->search_res.conn_id);
			break;
		}


		default:
			break;
	}
} // dumpGattClientEvent


void BLEUtils::dumpGapEvent(
	esp_gap_ble_cb_event_t event,
	esp_ble_gap_cb_param_t *param) {
	ESP_LOGD(LOG_TAG, "Received a GAP event: %s", gapEventToString(event).c_str());
	switch(event) {
		case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT: {
			ESP_LOGD(LOG_TAG, "[status: %d]",	param->scan_rsp_data_cmpl.status);
			break;
		} // ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT

		case ESP_GAP_BLE_ADV_START_COMPLETE_EVT: {
			ESP_LOGD(LOG_TAG, "[status: %d]",	param->scan_start_cmpl.status);
			break;
		} // ESP_GAP_BLE_ADV_START_COMPLETE_EVT

		case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT: {
			ESP_LOGD(LOG_TAG, "[status: %d]",	param->scan_stop_cmpl.status);
			break;
		} // ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT

		case ESP_GAP_BLE_NC_REQ_EVT: {
			ESP_LOGD(LOG_TAG, "[bd_addr: %s, passkey: %d]",
				BLEUtils::addressToString(param->ble_security.key_notif.bd_addr).c_str(),
				param->ble_security.key_notif.passkey);
			break;
		} // ESP_GAP_BLE_NC_REQ_EVT

		case ESP_GAP_BLE_LOCAL_IR_EVT: {
			break;
		} // ESP_GAP_BLE_LOCAL_IR_EVT

		case ESP_GAP_BLE_LOCAL_ER_EVT: {
			break;
		} // ESP_GAP_BLE_LOCAL_ER_EVT

		case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
			ESP_LOGD(LOG_TAG, "[status: %d]",	param->scan_param_cmpl.status);
			break;
		} // ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT

		case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT: {
			ESP_LOGD(LOG_TAG, "[status: %d]", param->scan_start_cmpl.status);
			break;
		} // ESP_GAP_BLE_SCAN_START_COMPLETE_EVT


		case ESP_GAP_BLE_SEC_REQ_EVT: {
			ESP_LOGD(LOG_TAG, "[bd_addr: %s]", BLEUtils::addressToString(param->ble_security.ble_req.bd_addr).c_str());
			break;
		} // ESP_GAP_BLE_SEC_REQ_EVT

		case ESP_GAP_BLE_AUTH_CMPL_EVT: {
			ESP_LOGD(LOG_TAG, "[bd_addr: %s, key_present: %d, key: ***, key_type: %d, success: %d, fail_reason: %d, addr_type: ***, dev_type: %s]",
				BLEUtils::addressToString(param->ble_security.auth_cmpl.bd_addr).c_str(),
				param->ble_security.auth_cmpl.key_present,
				param->ble_security.auth_cmpl.key_type,
				param->ble_security.auth_cmpl.success,
				param->ble_security.auth_cmpl.fail_reason,
				BLEUtils::devTypeToString(param->ble_security.auth_cmpl.dev_type).c_str()
			);
			break;
		} // ESP_GAP_BLE_AUTH_CMPL_EVT

		default: {
			ESP_LOGD(LOG_TAG, "*** dumpGapEvent: Logger not coded ***")
			break;
		} // default
	} // switch
} // dumpGapEvent


/**
 * @brief Dump the details of a GATT server event.
 * A GATT Server event is a callback received from the BLE subsystem when we are acting as a BLE
 * server.  The callback indicates the type of event in the `event` field.  The `evtParam` is a
 * union of structures where we can use the `event` to indicate which of the structures has been
 * populated and hence is valid.
 *
 * @param [in] event The event type that was posted.
 * @param [in] evtParam A union of structures only one of which is populated.
 */
void BLEUtils::dumpGattServerEvent(
		esp_gatts_cb_event_t event,
		esp_gatt_if_t gatts_if,
		esp_ble_gatts_cb_param_t *evtParam) {
	ESP_LOGD(LOG_TAG, "GATT ServerEvent: %s", bt_utils_gatt_server_event_type_to_string(event).c_str());
	switch(event) {

		case ESP_GATTS_ADD_CHAR_DESCR_EVT: {
			ESP_LOGD(LOG_TAG, "[status: %s, attr_handle: 0x%.2x, service_handle: 0x%.2x, char_uuid: %s]",
				gattStatusToString(evtParam->add_char_descr.status).c_str(),
				evtParam->add_char_descr.attr_handle,
				evtParam->add_char_descr.service_handle,
				BLEUUID(evtParam->add_char_descr.char_uuid).toString().c_str());
			break;
		} // ESP_GATTS_ADD_CHAR_DESCR_EVT

		case ESP_GATTS_ADD_CHAR_EVT: {
			ESP_LOGD(LOG_TAG, "[status: %s, attr_handle: 0x%.2x, service_handle: 0x%.2x, char_uuid: %s]",
				gattStatusToString(evtParam->add_char.status).c_str(),
				evtParam->add_char.attr_handle,
				evtParam->add_char.service_handle,
				BLEUUID(evtParam->add_char.char_uuid).toString().c_str());
			break;
		} // ESP_GATTS_ADD_CHAR_EVT

		case ESP_GATTS_CONF_EVT: {
			ESP_LOGD(LOG_TAG, "[status: %s, conn_id: 0x%.2x]",
				gattStatusToString(evtParam->conf.status).c_str(),
				evtParam->conf.conn_id);
			break;
		} // ESP_GATTS_CONF_EVT

		case ESP_GATTS_CONGEST_EVT: {
			ESP_LOGD(LOG_TAG, "[conn_id: %d, congested: %d]",
				evtParam->congest.conn_id,
				evtParam->congest.congested);
		} // ESP_GATTS_CONGEST_EVT

		case ESP_GATTS_CONNECT_EVT: {
			ESP_LOGD(LOG_TAG, "[conn_id: %d, remote_bda: %s, is_connected: %d]",
				evtParam->connect.conn_id,
				addressToString(evtParam->connect.remote_bda).c_str(),
				evtParam->connect.is_connected);
			break;
		} // ESP_GATTS_CONNECT_EVT

		case ESP_GATTS_CREATE_EVT: {
			ESP_LOGD(LOG_TAG, "[status: %s, service_handle: 0x%.2x, service_id: [%s]]",
				gattStatusToString(evtParam->create.status).c_str(),
				evtParam->create.service_handle,
				gattServiceIdToString(evtParam->create.service_id).c_str());
			break;
		} // ESP_GATTS_CREATE_EVT

		case ESP_GATTS_DISCONNECT_EVT: {
			ESP_LOGD(LOG_TAG, "[conn_id: %d, remote_bda: %s, is_connected: %d]",
				evtParam->connect.conn_id,
				addressToString(evtParam->connect.remote_bda).c_str(),
				evtParam->connect.is_connected);
			break;
		} // ESP_GATTS_DISCONNECT_EVT

		case ESP_GATTS_EXEC_WRITE_EVT: {
			ESP_LOGD(LOG_TAG, "[conn_id: %d, trans_id: %d, bda: %s, exec_write_flag: 0x%.2x]",
				evtParam->exec_write.conn_id,
				evtParam->exec_write.trans_id,
				addressToString(evtParam->exec_write.bda).c_str(),
				evtParam->exec_write.exec_write_flag);
			break;
		} // ESP_GATTS_DISCONNECT_EVT

		case ESP_GATTS_MTU_EVT: {
			ESP_LOGD(LOG_TAG, "[conn_id: %d, mtu: %d]",
					evtParam->mtu.conn_id,
					evtParam->mtu.mtu);
			break;
		} // ESP_GATTS_MTU_EVT

		case ESP_GATTS_READ_EVT: {
			ESP_LOGD(LOG_TAG, "[conn_id: %d, trans_id: %d, bda: %s, handle: 0x%.2x, is_long: %d, need_rsp:%d]",
					evtParam->read.conn_id,
					evtParam->read.trans_id,
					addressToString(evtParam->read.bda).c_str(),
					evtParam->read.handle,
					evtParam->read.is_long,
					evtParam->read.need_rsp);
			break;
		} // ESP_GATTS_READ_EVT

		case ESP_GATTS_RESPONSE_EVT: {
			ESP_LOGD(LOG_TAG, "[status: %s, handle: 0x%.2x]",
				gattStatusToString(evtParam->rsp.status).c_str(),
				evtParam->rsp.handle);
			break;
		} // ESP_GATTS_RESPONSE_EVT

		case ESP_GATTS_REG_EVT: {
			ESP_LOGD(LOG_TAG, "[status: %s, app_id: %d]",
				gattStatusToString(evtParam->reg.status).c_str(),
				evtParam->reg.app_id);
			break;
		} // ESP_GATTS_REG_EVT

		case ESP_GATTS_START_EVT: {
			ESP_LOGD(LOG_TAG, "[status: %s, service_handle: 0x%.2x]",
				gattStatusToString(evtParam->start.status).c_str(),
				evtParam->start.service_handle);
			break;
		} // ESP_GATTS_START_EVT

		case ESP_GATTS_WRITE_EVT: {
			ESP_LOGD(LOG_TAG, "[conn_id: %d, trans_id: %d, bda: %s, handle: 0x%.2x, offset: %d, need_rsp: %d, is_prep: %d, len: %d]",
					evtParam->write.conn_id,
					evtParam->write.trans_id,
					addressToString(evtParam->write.bda).c_str(),
					evtParam->write.handle,
					evtParam->write.offset,
					evtParam->write.need_rsp,
					evtParam->write.is_prep,
					evtParam->write.len);
			break;
		} // ESP_GATTS_WRITE_EVT

		default:
			ESP_LOGD(LOG_TAG, "dumpGattServerEvent: *** NOT CODED ***");
			break;
		}
} // dumpGattServerEvent

/**
 * @brief Convert a BLE address to a string.
 *
 * @param [in] bda The natural representation of an address.
 * @return The string representation of the address.
 */
std::string BLEUtils::addressToString(ble_address address) {
	std::stringstream stream;
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)address[0] << ':';
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)address[1] << ':';
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)address[2] << ':';
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)address[3] << ':';
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)address[4] << ':';
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)address[5];
	return stream.str();
}


/**
 * @brief Convert a BLE address to a string.
 *
 * @param [in] bda The natural representation of an address.
 * @return The string representation of the address.
 */
std::string BLEUtils::addressToString(esp_bd_addr_t address) {
	std::stringstream stream;
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)((uint8_t *)(address))[0] << ':';
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)((uint8_t *)(address))[1] << ':';
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)((uint8_t *)(address))[2] << ':';
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)((uint8_t *)(address))[3] << ':';
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)((uint8_t *)(address))[4] << ':';
	stream << std::setfill('0') << std::setw(2) << std::hex << (int)((uint8_t *)(address))[5];
	return stream.str();
}

esp_bt_uuid_t BLEUtils::buildUUID(uint16_t uuid) {
	esp_bt_uuid_t retUUID;
	retUUID.len = ESP_UUID_LEN_16;
	retUUID.uuid.uuid16 = uuid;
	return retUUID;
}

esp_bt_uuid_t BLEUtils::buildUUID(uint32_t uuid) {
	esp_bt_uuid_t retUUID;
	retUUID.len = ESP_UUID_LEN_32;
	retUUID.uuid.uuid32 = uuid;
	return retUUID;
}

/**
 * @brief Build a UUID.
 *
 * There are times when we wish to parse a string representation of a UUID into a more natural
 * form.  This function parses such a string and returns the ESP32 type called `esp_bt_uuid_t`.
 *
 * The format of string UUIDs is:
 * * 16bit  - XXXX
 * * 32bit  - XXXXXXXXX
 * * 128bit - Not yet implemented
 */
esp_bt_uuid_t BLEUtils::buildUUID(std::string uuid) {
	esp_bt_uuid_t retUUID;
	retUUID.len = 0;
	int charcterCount = uuid.length();
	if (charcterCount != 4 && charcterCount != 8) {
		ESP_LOGE(LOG_TAG, "BLEUtils::parseUUID: Malformed UUID string: %s", uuid.c_str());
		return retUUID;
	}
	if (charcterCount == 4) {
		retUUID.len = ESP_UUID_LEN_16;
		sscanf(uuid.c_str(), "%hx", &retUUID.uuid.uuid16);
	} else if (charcterCount == 8) {
		retUUID.len = ESP_UUID_LEN_32;
		sscanf(uuid.c_str(), "%x", &retUUID.uuid.uuid32);
	}
	return retUUID;
} // buildUUID

/**
 * @brief Find a %BLEDevice by a address.
 *
 * We keep can keep a record of BLEDevices by their address.  We can use this method
 * to retrieve the %BLEDevice that was previously saved by its address.
 *
 * @param [in] address The address of the device we want to locate.
 * @return A pointer to the %BLEDevice associated with this address.
 */
BLEDevice* BLEUtils::findByAddress(ble_address address) {
	ESP_LOGD(LOG_TAG, "findByAddress(%s)", addressToString(address).c_str());
	return g_addressMap.at(address);
} // findByAddress


/**
 * @brief Find a %BLEDevice by a conn_id.
 *
 * We keep can keep a record of BLEDevices by their conn_id.  We can use this method
 * to retrieve the %BLEDevice that was previously saved by its conn_id.
 *
 * @param [in] conn_id The conn_id of the device we want to locate.
 * @return A pointer to the %BLEDevice associated with this conn_id.
 */
BLEDevice* BLEUtils::findByConnId(uint16_t conn_id) {
	//try {
	ESP_LOGD(LOG_TAG, "findByConnId(%d)", conn_id);
	return g_connIdMap.at(conn_id);
	/*
	} catch(std::out_of_range &e) {
		ESP_LOGD(tag, "findByConnId: Not found %d", conn_id);
		return nullptr;
	}
	*/
} // findByConnId


/**
 * @brief Convert an esp_gatt_srvc_id_t to a string.
 */
std::string BLEUtils::gattServiceIdToString(esp_gatt_srvc_id_t srvcId) {
	return gattIdToString(srvcId.id);
} // gattServiceIdToString

/**
 * @brief Convert a string to a %BLE address.
 *
 * @param [in] uuid String representation of an address.
 * @return The natural representation of an address.
 */
ble_address BLEUtils::parseAddress(std::string uuid) {
	// ff:ff:45:19:14:80
	// 12345678901234567
	if (uuid.length() != 17) {
		return "";
	}

	int data[6];
	uint8_t b[6];
	sscanf(uuid.c_str(), "%x:%x:%x:%x:%x:%x", &data[0], &data[1], &data[2], &data[3], &data[4], &data[5]);
	b[0] = (uint8_t)data[0];
	b[1] = (uint8_t)data[1];
	b[2] = (uint8_t)data[2];
	b[3] = (uint8_t)data[3];
	b[4] = (uint8_t)data[4];
	b[5] = (uint8_t)data[5];

	return std::string((char *)b, 6);
} // parseUUID


/**
 * @brief Register a %BLEDevice by its address.
 *
 * Register a %BLEDevice by its address for subsequent lookup/retrieval.
 * @param [in] address The address of the device.
 * @param [in] pDevice A pointer to a %BLEDevice instance.
 */
void BLEUtils::registerByAddress(ble_address address, BLEDevice* pDevice) {
	ESP_LOGD(LOG_TAG, "registerByAddress(%s)", addressToString(address).c_str());
	g_addressMap.insert(std::pair<ble_address, BLEDevice *>(address, pDevice));
}

/**
 * @brief Register a %BLEDevice by its conn_id.
 *
 * Register a %BLEDevice by its conn_id for subsequent lookup/retrieval.
 * @param [in] address The conn_id of the device.
 * @param [in] pDevice A pointer to a %BLEDevice instance.
 */
void BLEUtils::registerByConnId(uint16_t conn_id, BLEDevice* pDevice) {
	ESP_LOGD(LOG_TAG, "registerByConnId(%d)", conn_id);
	g_connIdMap.insert(std::pair<uint16_t, BLEDevice *>(conn_id, pDevice));
} // registerByConnId



#endif // CONFIG_BT_ENABLED
