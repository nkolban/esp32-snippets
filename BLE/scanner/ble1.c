# include <bt.h>
#include <esp_bt_main.h>
#include <esp_gap_ble_api.h>
#include <esp_log.h>
#include <esp_blufi_api.h>
#include <bt_types.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "nvs_flash.h"
#include "sdkconfig.h"

#define BT_BD_ADDR_STR         "%02x:%02x:%02x:%02x:%02x:%02x"
#define BT_BD_ADDR_HEX(addr)   addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]

// Prototypes
static const char *bt_event_type_to_string(uint32_t eventType);
static const char *bt_gap_search_event_type_to_string(uint32_t searchEvt);
static const char *bt_addr_t_to_string(esp_ble_addr_type_t type);
static const char *bt_dev_type_to_string(esp_bt_dev_type_t type);
static void gap_callback_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static const char *btsig_gap_type(uint32_t gap_type);

#define tag "ble1"


static uint32_t convertU16ToU32(uint16_t inShortA, uint16_t inShortB) 
{
	return inShortA<< 16 | inShortB;
}


static uint16_t convertU8ToU16(uint8_t inByteA, uint8_t inByteB)
{
	return inByteA<<8 | inByteB;
}


// https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.service.health_thermometer.xml
static double decode1809(uint8_t  *payload)
{
	return ((float)(payload[4]<<16 | payload[3]<<8 | payload[2])) / 100.0;
}

static uint8_t decode180f(uint8_t  *payload)
{
	return payload[2];
}

static void dump_16bituuid(uint8_t  *payload,uint8_t length)
{
	//Find UUID from the two first bytes
	uint16_t uuid =0;

	uuid = convertU8ToU16 (payload[1],payload[0]);
	ESP_LOGD(tag,"Dump16 UUID %04X, len %d", uuid,length);


	length -=2; //Reduce with header size

	// A list of all GATT Services is here https://www.bluetooth.com/specifications/gatt/services
	switch(uuid)
	{
		
		case 0x1809:  // Health Thermometer
			if (length >=4) //Validate input payload length;
			{				
				ESP_LOGI(tag,"@ 0x1809 Temperature %f", decode1809(payload));
			}
			
			break;
		case 0x180f : //Battery Service (mandatory)
			if (length >=1)
			{ 
				ESP_LOGI(tag,"@ 0x180F Battery %d %%", decode180f(payload));
			}
			break;

		default:
			ESP_LOGI(tag,"@ 16 BIT UUID 0x%04X - Packet decoding for thtis type not implemented",uuid); // Read the Bluetooth spec and implement it
			break;
	}
}


void bin_to_strhex(unsigned char *bin, unsigned int binsz, char **result)
{
  char          hex_str[]= "0123456789abcdef";
  unsigned int  i;

  *result = (char *)malloc(binsz * 2 + 1);
  (*result)[binsz * 2] = 0;

  if (!binsz)
    return;

  for (i = 0; i < binsz; i++)
    {
      (*result)[i * 2 + 0] = hex_str[(bin[i] >> 4) & 0x0F];
      (*result)[i * 2 + 1] = hex_str[(bin[i]     ) & 0x0F];
    }  
}

static uint8_t dump_adv_payload(uint8_t *payload) 
{
	uint8_t length;
	uint8_t ad_type;
	uint8_t sizeConsumed = 0;
	int finished = 0;
	uint8_t total_length=0;
	//sprintf(text, "%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x")

	// GAP assigned numbers for the Type in the payload is defined here:
	// https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile

	while(!finished) {
		length = *payload;
		payload++;
		if (length != 0) {
			ad_type = *payload;
			
			ESP_LOGI(tag, "# Payload type: 0x%.2x (%s), length: %d", ad_type, btsig_gap_type(ad_type), length);			


			// Decode packets - implemented just a few types
			switch(ad_type)
			{
				case 0x16: // 16bit UUID - Bluetooth Core Specification:Vol. 3, Part C, sections 11.1.10 and 18.10 (v4.0)
					dump_16bituuid(payload+1, length);
					break;

				case 0x09:  //Complete local name - Bluetooth Core Specification:Vol. 3, Part C, section 8.1.2 (v2.1 + EDR, 3.0 + HS and 4.0)Vol. 3, Part C, sections 11.1.2 and 18.4 (v4.0)Core Specification Supplement, Part A, section 1.2
					ESP_LOGI(tag, "# Complete local name: %.*s", length, payload);			
					break;

				default:
					break;
			}

			// Dump the raw HEX data
			// Also dump as string, if possible, to make it possible to scan for text
			int i;
			int size = length / sizeof(char);
			char *hex_str = (char*) calloc(3 * size,1);
			char *ascii_str = (char*) calloc(length+1,1); 
			char *buf_ptr1 = hex_str;
			char *buf_ptr2 = ascii_str;

			unsigned char *source = (unsigned char *)payload+1;	

			if ((hex_str) && (ascii_str)) 
			{
				for (i = 0; i < size; i++)
				{
					buf_ptr1 += sprintf(buf_ptr1, i < size - 1 ? "%02X:" : "%02X", source[i]);

					
					char ch = source[i];
					
					//quick fix since isalpha had unexpected results
					int ichar = ((int) ch) & 0xFF;
					if ((ichar<32) || (ichar>126))
					{
						ch = '.';  // unprintable characters are represented as "."
					}
					
					buf_ptr2 += sprintf(buf_ptr2, "%c", ch);					
					
				}

				ESP_LOGI(tag,"* Payload: %s (%s)", hex_str, ascii_str);
			}
			if (hex_str) free(hex_str);
			hex_str=0;

			if (ascii_str) free(ascii_str);
			ascii_str=0;

			payload += length;
			total_length += length+1;
		}	

		sizeConsumed = 1 + length;
		if (sizeConsumed >=31 || length == 0) {
			finished = 1;
		}
	} // !finished
	return total_length;
} // dump_adv_payload


static void gap_callback_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
	ESP_LOGD(tag, "Received a GAP event: %s", bt_event_type_to_string(event));
	esp_ble_gap_cb_param_t *p = (esp_ble_gap_cb_param_t *)param;

	esp_err_t status;	


	switch (event) 
	{
		case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
		{
			ESP_LOGD(tag, "status: %d", p->scan_param_cmpl.status);
		
			// This procedure keep the device scanning the peer device which advertising on the air.
			//the unit of the duration is second
			uint32_t duration = 30;
			status = esp_ble_gap_start_scanning(duration); 
			if (status != ESP_OK) 
			{
				ESP_LOGE(tag, "esp_ble_gap_start_scanning: rc=%d", status);
			}
		}
		break;

		case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
		{
			//scan start complete event to indicate scan start successfully or failed
			if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
			{
          	  ESP_LOGE(tag, "Scan start failed");
			}
		}
		break;
		
		case ESP_GAP_BLE_SCAN_RESULT_EVT:
		{	
		
			ESP_LOGI(tag, "Device address (bda): %02x:%02x:%02x:%02x:%02x:%02x", BT_BD_ADDR_HEX(p->scan_rst.bda));
			
			ESP_LOGI(tag, "Device type         : %s", bt_dev_type_to_string(p->scan_rst.dev_type));
			ESP_LOGI(tag, "Search_evt          : %s", bt_gap_search_event_type_to_string(p->scan_rst.search_evt));
			ESP_LOGI(tag, "Addr_type           : %s", bt_addr_t_to_string(p->scan_rst.ble_addr_type));
			ESP_LOGI(tag, "RSSI                : %d", p->scan_rst.rssi);
			ESP_LOGI(tag, "Flag                : %d", p->scan_rst.flag);
			  
			//bit 0 (OFF) LE Limited Discoverable Mode
   			//bit 1 (OFF) LE General Discoverable Mode
   			//bit 2 (ON) BR/EDR Not Supported
   			//bit 3 (OFF) Simultaneous LE and BR/EDR to Same Device Capable (controller)
   			//bit 4 (OFF) Simultaneous LE and BR/EDR to Same Device Capable (Host)
			
		  	ESP_LOGI(tag, "num_resps           : %d", p->scan_rst.num_resps);

			if ( p->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_CMPL_EVT)
			{
				// Scan is done.

				// The next 5 codelines automatically restarts the scan. You you only want
				// one scan round, you can comment it.
				uint32_t duration = 30;
				status = esp_ble_gap_start_scanning	(duration); 
				if (status != ESP_OK) 
				{
					ESP_LOGE(tag, "esp_ble_gap_start_scanning: rc=%d", status);
				}

				return;
			}

			
			if (p->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_RES_EVT) 
			{

				// NOTE!
				// Instead of this code that dumps the hole payload
				// you can search for elements in the payload using the
				// function esp_ble_resolve_adv_data()
				//
				// Like this, that scans for the "Complete name" (looking inside the payload buffer)
				/*
				uint8_t len;
				uint8_t *data = esp_ble_resolve_adv_data(p->scan_rst.ble_adv, ESP_BLE_AD_TYPE_NAME_CMPL, &len);
				ESP_LOGD(tag, "len: %d, %.*s", len, len, data);
				*/

				uint8_t length;
				length = dump_adv_payload(p->scan_rst.ble_adv);
				ESP_LOGI(tag, "Payload total length: %d", length);
			}
			ESP_LOGI(tag, "");
			
		}
		break;

		case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
		{
			if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
			{
				ESP_LOGE(tag, "Scan stop failed");
			}
			else 
			{
				ESP_LOGI(tag, "Stop scan successfully");
			}
		}

		case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
		{
			if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
			{
				ESP_LOGE(tag, "Adv stop failed");
			}
			else 
			{
				ESP_LOGI(tag, "Stop adv successfully");
			}
		}
		break;
		
		
		default:
        break;
	}
} // gap_callback_handler

static const char *bt_dev_type_to_string(esp_bt_dev_type_t type) {
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

static const char *bt_addr_t_to_string(esp_ble_addr_type_t type) {
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

static const char *bt_gap_search_event_type_to_string(uint32_t searchEvt) {
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
			return "Unknown event type";
	}
} // bt_gap_search_event_type_to_string

/*
 * Convert a BT GAP event type to a string representation.
 */
static const char *bt_event_type_to_string(uint32_t eventType) {
	switch(eventType) {
		case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
			return "ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT";
		case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
			return "ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT";
		case ESP_GAP_BLE_SCAN_RESULT_EVT:
			return "ESP_GAP_BLE_SCAN_RESULT_EVT";
		case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
			return "ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT";
		default:
			return "Unknown event type";
	}
} // bt_event_type_to_string

static const char *btsig_gap_type(uint32_t gap_type) {
	switch (gap_type)
	{
		case 0x01: return "Flags";
		case 0x02: return "Incomplete List of 16-bit Service Class UUIDs";
		case 0x03: return "Complete List of 16-bit Service Class UUIDs";
		case 0x04: return "Incomplete List of 32-bit Service Class UUIDs";
		case 0x05: return "Complete List of 32-bit Service Class UUIDs";
		case 0x06: return "Incomplete List of 128-bit Service Class UUIDs";
		case 0x07: return "Complete List of 128-bit Service Class UUIDs";
		case 0x08: return "Shortened Local Name";
		case 0x09: return "Complete Local Name";
		case 0x0A: return "Tx Power Level";
		case 0x0D: return "Class of Device";
		case 0x0E: return "Simple Pairing Hash C/C-192";
		case 0x0F: return "Simple Pairing Randomizer R/R-192";
		case 0x10: return "Device ID/Security Manager TK Value";
		case 0x11: return "Security Manager Out of Band Flags";
		case 0x12: return "Slave Connection Interval Range";
		case 0x14: return "List of 16-bit Service Solicitation UUIDs";
		case 0x1F: return "List of 32-bit Service Solicitation UUIDs";
		case 0x15: return "List of 128-bit Service Solicitation UUIDs";
		case 0x16: return "Service Data - 16-bit UUID";
		case 0x20: return "Service Data - 32-bit UUID";
		case 0x21: return "Service Data - 128-bit UUID";
		case 0x22: return "LE Secure Connections Confirmation Value";
		case 0x23: return "LE Secure Connections Random Value";
		case 0x24: return "URI";
		case 0x25: return "Indoor Positioning";
		case 0x26: return "Transport Discovery Data";
		case 0x17: return "Public Target Address";
		case 0x18: return "Random Target Address";
		case 0x19: return "Appearance";
		case 0x1A: return "Advertising Interval";
		case 0x1B: return "LE Bluetooth Device Address";
		case 0x1C: return "LE Role";
		case 0x1D: return "Simple Pairing Hash C-256";
		case 0x1E: return "Simple Pairing Randomizer R-256";
		case 0x3D: return "3D Information Data";
		case 0xFF: return "Manufacturer Specific Data";
		
		default: 
			return "Unknown type";
	}
}

	
esp_err_t register_ble_functionality(void)
{
	esp_err_t status;	
	
	ESP_LOGI(tag, "Register GAP callback");
	
	// This function is called to occur gap event, such as scan result.
	//register the scan callback function to the gap module
	status = esp_ble_gap_register_callback(gap_callback_handler);
	if (status != ESP_OK) 
	{
		ESP_LOGE(tag, "esp_ble_gap_register_callback: rc=%d", status);
		return ESP_FAIL;
	}

	static esp_ble_scan_params_t ble_scan_params = 
	{	
		.scan_type              = BLE_SCAN_TYPE_ACTIVE,
		.own_addr_type          = BLE_ADDR_TYPE_PUBLIC,
		.scan_filter_policy     = BLE_SCAN_FILTER_ALLOW_ALL,
		.scan_interval          = 0x50,
		.scan_window            = 0x30		
	};

	ESP_LOGI(tag, "Set GAP scan parameters");
	
	// This function is called to set scan parameters.	
	status = esp_ble_gap_set_scan_params(&ble_scan_params);
	if (status != ESP_OK) 
	{
		ESP_LOGE(tag, "esp_ble_gap_set_scan_params: rc=%d", status);
		return ESP_FAIL;
	}


	ESP_LOGD(tag, "We have registered what we need!");

	return ESP_OK ;
}

// Main start code running in its own Xtask
void bt_task(void *ignore)
{
	esp_err_t status;

	// Initialize NVS flash storage with layout given in the partition table
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES) 
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK( ret );

	ESP_LOGI(tag, "Enabling Bluetooth Controller");
	
	// Initialize BT controller to allocate task and other resource. 
	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	if (esp_bt_controller_init(&bt_cfg) != ESP_OK) 
	{
		ESP_LOGE(tag, "Bluetooth controller initialize failed");
		goto end; 
	}

	// Enable BT controller
	if (esp_bt_controller_enable(ESP_BT_MODE_BTDM) != ESP_OK) 
	{
		ESP_LOGE(tag, "Bluetooth controller enable failed");
		goto end; 
	}

	ESP_LOGI(tag, "Bluetooth Controller Enabled");

	ESP_LOGI(tag, "Init Bluetooth stack");
	
	// Init and alloc the resource for bluetooth, must be prior to every bluetooth stuff
	status = esp_bluedroid_init(); 
	if (status != ESP_OK)
	{ 
		ESP_LOGE(tag, "%s init bluetooth failed\n", __func__); 
		goto end; 
	} 
	
	// Enable bluetooth, must after esp_bluedroid_init()
	status = esp_bluedroid_enable(); 
	if (status != ESP_OK) 
	{ 
		ESP_LOGE(tag, "%s enable bluetooth failed\n", __func__); 
		goto end;
	} 

	ESP_LOGI(tag, "Bluetooth stack initialized");

	ESP_LOGI(tag, "Register BLE functionality");
	status = register_ble_functionality();
	if (status != ESP_OK)
	{
		ESP_LOGE(tag, "Register BLE functionality failed");
		goto end;
	}

	while(1)
    {
        vTaskDelay(1000);
    }

end:
	ESP_LOGI(tag, "Terminating BT logging task");
	vTaskDelete(NULL);
	
	
} // bt_task
