#include <esp_log.h>
#include <esp_err.h>
#include <esp_system.h>
#include <esp_event.h>
#include <esp_event_loop.h>
#include <esp_wifi.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <mongoose.h>
#include "bootwifi.h"
#include "sdkconfig.h"
#include "selectAP.h"
// Key used in NVS for connection info
#define KEY_CONNECTION_INFO "connectionInfo"

// Namespace in NVS for bootwifi
#define BOOTWIFI_NAMESPACE "bootwifi"

#define SSID_SIZE (32)
#define PASSWORD_SIZE (64)

#define OVERRIDE_GPIO GPIO_NUM_25

typedef struct {
	char ssid[SSID_SIZE];
	char password[PASSWORD_SIZE];
} connection_info_t;

static char tag[] = "bootwifi";

static bootwifi_callback_t g_callback = NULL;

static int g_mongooseStarted = 0; // Has the mongoose server started?
static int g_mongooseStopRequest = 0; // Request to stop the mongoose server.

static void saveConnectionInfo(connection_info_t *pConnectionInfo);
static void becomeAccessPoint();
static void bootWiFi2();

/**
 * Convert a Mongoose event type to a string.
 */
static char *mongoose_eventToString(int ev) {
	static char temp[100];
	switch (ev) {
	case MG_EV_CONNECT:
		return "MG_EV_CONNECT";
	case MG_EV_ACCEPT:
		return "MG_EV_ACCEPT";
	case MG_EV_CLOSE:
		return "MG_EV_CLOSE";
	case MG_EV_SEND:
		return "MG_EV_SEND";
	case MG_EV_RECV:
		return "MG_EV_RECV";
	case MG_EV_HTTP_REQUEST:
		return "MG_EV_HTTP_REQUEST";
	case MG_EV_MQTT_CONNACK:
		return "MG_EV_MQTT_CONNACK";
	case MG_EV_MQTT_CONNACK_ACCEPTED:
		return "MG_EV_MQTT_CONNACK";
	case MG_EV_MQTT_CONNECT:
		return "MG_EV_MQTT_CONNECT";
	case MG_EV_MQTT_DISCONNECT:
		return "MG_EV_MQTT_DISCONNECT";
	case MG_EV_MQTT_PINGREQ:
		return "MG_EV_MQTT_PINGREQ";
	case MG_EV_MQTT_PINGRESP:
		return "MG_EV_MQTT_PINGRESP";
	case MG_EV_MQTT_PUBACK:
		return "MG_EV_MQTT_PUBACK";
	case MG_EV_MQTT_PUBCOMP:
		return "MG_EV_MQTT_PUBCOMP";
	case MG_EV_MQTT_PUBLISH:
		return "MG_EV_MQTT_PUBLISH";
	case MG_EV_MQTT_PUBREC:
		return "MG_EV_MQTT_PUBREC";
	case MG_EV_MQTT_PUBREL:
		return "MG_EV_MQTT_PUBREL";
	case MG_EV_MQTT_SUBACK:
		return "MG_EV_MQTT_SUBACK";
	case MG_EV_MQTT_SUBSCRIBE:
		return "MG_EV_MQTT_SUBSCRIBE";
	case MG_EV_MQTT_UNSUBACK:
		return "MG_EV_MQTT_UNSUBACK";
	case MG_EV_MQTT_UNSUBSCRIBE:
		return "MG_EV_MQTT_UNSUBSCRIBE";
	case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST:
		return "MG_EV_WEBSOCKET_HANDSHAKE_REQUEST";
	case MG_EV_WEBSOCKET_HANDSHAKE_DONE:
		return "MG_EV_WEBSOCKET_HANDSHAKE_DONE";
	case MG_EV_WEBSOCKET_FRAME:
		return "MG_EV_WEBSOCKET_FRAME";
	}
	sprintf(temp, "Unknown event: %d", ev);
	return temp;
} //eventToString


// Convert a Mongoose string type to a string.
static char *mgStrToStr(struct mg_str mgStr) {
	char *retStr = (char *) malloc(mgStr.len + 1);
	memcpy(retStr, mgStr.p, mgStr.len);
	retStr[mgStr.len] = 0;
	return retStr;
} // mgStrToStr


/**
 * Handle mongoose events.  These are mostly requests to process incoming
 * browser requests.  The ones we handle are:
 * GET / - Send the enter details page.
 * GET /set - Set the connection info (REST request).
 * POST /ssidSelected - Set the connection info (HTML FORM).
 */
static void mongoose_event_handler(struct mg_connection *nc, int ev, void *evData) {
	ESP_LOGD(tag, "- Event: %s", mongoose_eventToString(ev));
	switch (ev) {
		case MG_EV_HTTP_REQUEST: {
			struct http_message *message = (struct http_message *) evData;
			char *uri = mgStrToStr(message->uri);
			ESP_LOGD(tag, " - uri: %s", uri);

			if (strcmp(uri, "/set") ==0 ) {
				connection_info_t connectionInfo;
//fix
				saveConnectionInfo(&connectionInfo);
				ESP_LOGD(tag, "- Set the new connection info to ssid: %s, password: %s",
					connectionInfo.ssid, connectionInfo.password);
				mg_send_head(nc, 200, 0, "Content-Type: text/plain");
			} if (strcmp(uri, "/") == 0) {
				mg_send_head(nc, 200, sizeof(selectAP_html), "Content-Type: text/html");
				mg_send(nc, selectAP_html, sizeof(selectAP_html));
			} if(strcmp(uri, "/ssidSelected") == 0) {
				// We have received a form page containing the details.  The form body will
				// contain:
				// ssid=<value>&password=<value>
				ESP_LOGD(tag, "- body: %.*s", message->body.len, message->body.p);
				connection_info_t connectionInfo;
				mg_get_http_var(&message->body, "ssid",
					connectionInfo.ssid, SSID_SIZE);
				mg_get_http_var(&message->body, "password",
					connectionInfo.password, PASSWORD_SIZE);
				ESP_LOGD(tag, "ssid: %s, password: %s", connectionInfo.ssid, connectionInfo.password);
				mg_send_head(nc, 200, 0, "Content-Type: text/plain");
				saveConnectionInfo(&connectionInfo);
				bootWiFi2();
			}	else {
				mg_send_head(nc, 404, 0, "Content-Type: text/plain");
			}
			nc->flags |= MG_F_SEND_AND_CLOSE;
			free(uri);
			break;
		} // MG_EV_HTTP_REQUEST
	} // End of switch
} // End of mongoose_event_handler


// FreeRTOS task to start Mongoose.
static void mongooseTask(void *data) {
	struct mg_mgr mgr;
	struct mg_connection *connection;

	ESP_LOGD(tag, ">> mongooseTask");
	g_mongooseStopRequest = 0; // Unset the stop request since we are being asked to start.

	mg_mgr_init(&mgr, NULL);

	connection = mg_bind(&mgr, ":80", mongoose_event_handler);

	if (connection == NULL) {
		ESP_LOGE(tag, "No connection from the mg_bind().");
		mg_mgr_free(&mgr);
		ESP_LOGD(tag, "<< mongooseTask");
		vTaskDelete(NULL);
		return;
	}
	mg_set_protocol_http_websocket(connection);

	// Keep processing until we are flagged that there is a stop request.
	while (!g_mongooseStopRequest) {
		mg_mgr_poll(&mgr, 1000);
	}

	// We have received a stop request, so stop being a web server.
	mg_mgr_free(&mgr);
	g_mongooseStarted = 0;

	ESP_LOGD(tag, "<< mongooseTask");
	vTaskDelete(NULL);
	return;
} // mongooseTask


/**
 * An ESP32 WiFi event handler.
 * The types of events that can be received here are:
 *
 * SYSTEM_EVENT_AP_PROBEREQRECVED
 * SYSTEM_EVENT_AP_STACONNECTED
 * SYSTEM_EVENT_AP_STADISCONNECTED
 * SYSTEM_EVENT_AP_START
 * SYSTEM_EVENT_AP_STOP
 * SYSTEM_EVENT_SCAN_DONE
 * SYSTEM_EVENT_STA_AUTHMODE_CHANGE
 * SYSTEM_EVENT_STA_CONNECTED
 * SYSTEM_EVENT_STA_DISCONNECTED
 * SYSTEM_EVENT_STA_GOT_IP
 * SYSTEM_EVENT_STA_START
 * SYSTEM_EVENT_STA_STOP
 * SYSTEM_EVENT_WIFI_READY
 */
static esp_err_t esp32_wifi_eventHandler(void *ctx, system_event_t *event) {
	// Your event handling code here...
	switch(event->event_id) {
		// When we have started being an access point, then start being a web server.
		case SYSTEM_EVENT_AP_START: { // Handle the AP start event
			ESP_LOGD(tag, "AP started");
			// Start Mongoose ...
			if (!g_mongooseStarted)
			{
				g_mongooseStarted = 1;
				xTaskCreatePinnedToCore(&mongooseTask, "mongoose_task", 2048, NULL, 5, NULL, 0);
			}
			break;
		} // SYSTEM_EVENT_AP_START

		// If we fail to connect to an access point as a station, become an access point.
		case SYSTEM_EVENT_STA_DISCONNECTED: {
			// We think we tried to connect as a station and failed! ... become
			// an access point.
			becomeAccessPoint();
			break;
		} // SYSTEM_EVENT_AP_START

		// If we connected as a station then we are done and we can stop being a
		// web server.
		case SYSTEM_EVENT_STA_GOT_IP: {
			ESP_LOGD(tag, " *******************************************");
			ESP_LOGD(tag, "* We are now connected and ready to do work!")
			ESP_LOGD(tag, "* - Our IP address is: " IPSTR, IP2STR(&event->event_info.got_ip.ip_info.ip));
			ESP_LOGD(tag, " *******************************************");
			g_mongooseStopRequest = 1; // Stop mongoose (if it is running).
			if (g_callback) {
				g_callback(1);
			}
			break;
		} // SYSTEM_EVENT_STA_GOTIP

		default: // Ignore the other event types
			break;
	} // Switch event

	return ESP_OK;
} // esp32_wifi_eventHandler


/**
 * Retrieve the connection info.  A rc==0 means ok.
 */
static int getConnectionInfo(connection_info_t *pConnectionInfo) {
	nvs_handle handle;
	size_t size;
	esp_err_t err;
	err = nvs_open(BOOTWIFI_NAMESPACE, NVS_READWRITE, &handle);
	if (err != 0) {
		ESP_LOGE(tag, "nvs_open: %x", err);
		return -1;
	}

	size = sizeof(connection_info_t);
	err = nvs_get_blob(handle, KEY_CONNECTION_INFO, pConnectionInfo, &size);
	if (err == ESP_ERR_NVS_NOT_FOUND) {
		nvs_close(handle);
		return -1;
	}
	if (err != 0) {
		ESP_LOGE(tag, "nvs_open: %x", err);
		nvs_close(handle);
		return -1;
	}
	nvs_close(handle);
	return 0;
} // getConnectionInfo


/**
 * Save our connection info for retrieval on a subsequent restart.
 */
static void saveConnectionInfo(connection_info_t *pConnectionInfo) {
	nvs_handle handle;
	ESP_ERROR_CHECK(nvs_open(BOOTWIFI_NAMESPACE, NVS_READWRITE, &handle));
	ESP_ERROR_CHECK(nvs_set_blob(handle, KEY_CONNECTION_INFO, pConnectionInfo,
			sizeof(connection_info_t)));
	ESP_ERROR_CHECK(nvs_commit(handle));
	nvs_close(handle);
} // setConnectionInfo


/**
 * Become a station connecting to an existing access point.
 */
static void becomeStation(connection_info_t *pConnectionInfo) {
	ESP_LOGD(tag, "- Connecting to access point %s ...", pConnectionInfo->ssid);

  ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA));
  wifi_config_t sta_config;
  sta_config.sta.bssid_set = 0;
  memcpy(sta_config.sta.ssid, pConnectionInfo->ssid, SSID_SIZE);
  memcpy(sta_config.sta.password, pConnectionInfo->password, PASSWORD_SIZE);
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_ERROR_CHECK(esp_wifi_connect());
} // becomeStation


/**
 * Become an access point.
 */
static void becomeAccessPoint() {
	ESP_LOGD(tag, "- Starting being an access point ...");
	// We don't have connection info so be an access point!
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	wifi_config_t apConfig = {
		.ap = {
			.ssid="ESP32 Duktape",
			.ssid_len=0,
			.password="Duktape",
			.channel=0,
			.authmode=WIFI_AUTH_OPEN,
			.ssid_hidden=0,
			.max_connection=4,
			.beacon_interval=100
		}
	};
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &apConfig));
	ESP_ERROR_CHECK(esp_wifi_start());
} // becomeAccessPoint


/**
 * Retrieve the signal level on the OVERRIDE_GPIO pin.  This is used to
 * indicate that we should not attempt to connect to any previously saved
 * access point we may know about.
 */
static int checkOverrideGpio() {
	gpio_pad_select_gpio(OVERRIDE_GPIO);
	gpio_set_direction(OVERRIDE_GPIO, GPIO_MODE_INPUT);
	gpio_set_pull_mode(OVERRIDE_GPIO, GPIO_PULLDOWN_ONLY);
	return gpio_get_level(OVERRIDE_GPIO);
} // checkOverrideGpio


static void bootWiFi2() {
	ESP_LOGD(tag, ">> bootWiFi2");
	// Check for a GPIO override which occurs when a physical Pin is high
	// during the test.  This can force the ability to check for new configuration
	// even if the existing configured access point is available.
	if (checkOverrideGpio()) {
		ESP_LOGD(tag, "- GPIO override detected");
		becomeAccessPoint();
	} else {
		// There was NO GPIO override, proceed as normal.  This means we retrieve
		// our stored access point information of the access point we should connect
		// against.  If that information doesn't exist, then again we become an
		// access point ourselves in order to allow a client to connect and bring
		// up a browser.
		connection_info_t connectionInfo;
		int rc = getConnectionInfo(&connectionInfo);
		if (rc == 0) {
			// We have received connection information, let us now become a station
			// and attempt to connect to the access point.
			becomeStation(&connectionInfo);

		} else {
			// We do NOT have connection information.  Let us now become an access
			// point that serves up a web server and allow a browser user to specify
			// the details that will be eventually used to allow us to connect
			// as a station.
			becomeAccessPoint();
		} // We do NOT have connection info
	}
	ESP_LOGD(tag, "<< bootWiFi2");
} // bootWiFi2


/**
 * Main entry into bootWiFi
 */
void bootWiFi(bootwifi_callback_t callback) {
	ESP_LOGD(tag, ">> bootWiFi");
	g_callback = callback;
	nvs_flash_init();
	tcpip_adapter_init();
	ESP_ERROR_CHECK(esp_event_loop_init(esp32_wifi_eventHandler, NULL));
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

	bootWiFi2();

	ESP_LOGD(tag, "<< bootWiFi");
} // bootWiFi
