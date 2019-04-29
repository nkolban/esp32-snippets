/**
 * Bootwifi - Boot the WiFi environment.
 *
 * Compile with -DBOOTWIFI_OVERRIDE_GPIO=<num> where <num> is a GPIO pin number
 * to use a GPIO override.
 * See the README.md for full information.
 *
 */
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_err.h>
#include <nvs_flash.h>
#include <CPPNVS.h>
#include <driver/gpio.h>

//#include <lwip/sockets.h>

#include <WiFi.h>
#include <string.h>
#include <string>
#include <HttpServer.h>
#include <System.h>
#include <GeneralUtils.h>
#include "BootWiFi.h"
#include "JSON.h"
#if defined(RESTART_COUNTER)
# include <freertos/timers.h>
#endif
#include "esp_wifi.h"
#include "esp_err.h"
#include "sdkconfig.h"
#include "selectAP.h"

// If the structure of a record saved for a subsequent reboot changes
// then consider using semver to change the version number or else
// we may try and boot with the wrong data.
#define KEY_VERSION "version"
uint32_t g_version=0x0100;

#if defined(RESTART_COUNTER)
# define BOOTWIFI_RESTART_COUNT_RESET     (3)
# define BOOTWIFI_RESTART_TIMEOUT_MS      (3000)
# define KEY_RESTART         "restartCount"
#endif

#define KEY_CONNECTION_INFO "connectionInfo"   // Key used in NVS for connection info
#define BOOTWIFI_NAMESPACE  "bootwifi"         // Namespace in NVS for bootwifi
#define SSID_SIZE           (32)               // Maximum SSID size
#define PASSWORD_SIZE       (64)               // Maximum password size


/**
 * The information about a WiFi access point connection.
 */
typedef struct {
	char ssid[SSID_SIZE];
	char password[PASSWORD_SIZE];
	tcpip_adapter_ip_info_t ipInfo; // Optional static IP information
} connection_info_t;

//static bootwifi_callback_t g_callback = NULL; // Callback function to be invoked when we have finished.


// Forward declarations
static void saveConnectionInfo(connection_info_t *pConnectionInfo);
#if defined(RESTART_COUNTER)
static int restart_count_get(void);
#endif

static const char TAG[] = "bootwifi";

static std::vector<WiFiAPRecord> apList;

static void dumpConnectionInfo(connection_info_t *pConnectionInfo) {
	ESP_LOGD(TAG, "connection_info.ssid = %.*s",     SSID_SIZE,     pConnectionInfo->ssid);
	ESP_LOGD(TAG, "connection_info.password = %.*s", PASSWORD_SIZE, pConnectionInfo->password);
	ESP_LOGD(TAG, "ip: %s, gw: %s, netmask: %s",
		GeneralUtils::ipToString((uint8_t*)&pConnectionInfo->ipInfo.ip).c_str(),
		GeneralUtils::ipToString((uint8_t*)&pConnectionInfo->ipInfo.gw).c_str(),
		GeneralUtils::ipToString((uint8_t*)&pConnectionInfo->ipInfo.netmask).c_str());
}


/**
 * Retrieve the connection info.  A rc==0 means ok.
 */
static int getConnectionInfo(connection_info_t *pConnectionInfo) {
	ESP_LOGD(TAG, ">> getConnectionInfo");
	size_t     size;
	uint32_t   version;

	NVS myNamespace(BOOTWIFI_NAMESPACE);
	myNamespace.get(KEY_VERSION, version);

	// Check the versions match
	if ((version & 0xff00) != (g_version & 0xff00)) {
		ESP_LOGD(TAG, "Incompatible versions ... current is %x, found is %x", version, g_version);
		return -1;
	}

	size = sizeof(connection_info_t);
	myNamespace.get(KEY_CONNECTION_INFO, (uint8_t*)pConnectionInfo, size);

	// Do a sanity check on the SSID
	if (strlen(pConnectionInfo->ssid) == 0) {
		ESP_LOGD(TAG, "NULL ssid detected");
		return -1;
	}
	dumpConnectionInfo(pConnectionInfo);
	ESP_LOGD(TAG, "<< getConnectionInfo");
	return 0;

} // getConnectionInfo


/**
 * Save our connection info for retrieval on a subsequent restart.
 */
static void saveConnectionInfo(connection_info_t *pConnectionInfo) {
	dumpConnectionInfo(pConnectionInfo);
	NVS myNamespace(BOOTWIFI_NAMESPACE);
	myNamespace.set(KEY_CONNECTION_INFO, (uint8_t*)pConnectionInfo, sizeof(connection_info_t));
	myNamespace.set(KEY_VERSION, g_version);
	myNamespace.commit();
} // setConnectionInfo


/**
 * Retrieve the signal level on the OVERRIDE_GPIO pin.  This is used to
 * indicate that we should not attempt to connect to any previously saved
 * access point we may know about.
 */

static int checkOverrideGpio() {
#ifdef BOOTWIFI_OVERRIDE_GPIO
	gpio_pad_select_gpio(BOOTWIFI_OVERRIDE_GPIO);
	gpio_set_direction(BOOTWIFI_OVERRIDE_GPIO, GPIO_MODE_INPUT);
	gpio_set_pull_mode(BOOTWIFI_OVERRIDE_GPIO, GPIO_PULLDOWN_ONLY);
	return gpio_get_level(BOOTWIFI_OVERRIDE_GPIO);
#else
	return 0; // If no boot override, return false
#endif
} // checkOverrideGpio


static void sendForm(HttpRequest* pRequest, HttpResponse* pResponse) {
	pResponse->setStatus(HttpResponse::HTTP_STATUS_OK, "OK");
	pResponse->addHeader(HttpRequest::HTTP_HEADER_CONTENT_TYPE, "text/html");

	// send the prefix

	pResponse->sendData(formPrefix);

	// now create the SSID input

	std::string ssidInput =
			"<td>SSID:</td>\n"
			"<td><input type=\"text\" autocorrect=\"off\" autocapitalize=\"none\" name=\"ssid\" id=\"ssid\" list=\"ssidList\"/>\n"
			"<datalist id=\"ssidList\">\n"
	  	  	  "<label for=\"suggestion\">or pick a SSID</label>\n"
	  	  	  "<select id=\"suggestion\" name=\"altSsid\">\n";

	std::vector<WiFiAPRecord>::iterator it;
	for (it = apList.begin(); it != apList.end(); it++)
	{
		ssidInput += "<option>" + it->getSSID() + "</option>\n";
		ESP_LOGI(TAG, "%s, %d rssi", it->getSSID().c_str() , it->getRSSI());
	}

	ssidInput += "</select>\n</datalist>\n</td>\n";

	pResponse->sendData(ssidInput);

	// then send the postfix

	pResponse->sendData(formPostfix);

	// now close

	pResponse->close();

} // sendForm


static void copyData(uint8_t* pTarget, size_t targetLength, std::string source) {
	memset(pTarget, 0, targetLength);
	size_t copySize = (source.length() > targetLength)? targetLength:source.length();
	memcpy(pTarget, source.data(), copySize);
	if (copySize < targetLength) {
		pTarget[copySize] = '\0';
	}
} // copyData


/**
 * @brief Process the form response.
 */
static void processForm(HttpRequest* pRequest, HttpResponse* pResponse) {
	ESP_LOGD(TAG, ">> processForm");
	std::map<std::string, std::string> formMap = pRequest->parseForm();
	connection_info_t connectionInfo;
	copyData((uint8_t*)connectionInfo.ssid, SSID_SIZE, formMap["ssid"]);
	copyData((uint8_t*)connectionInfo.password, PASSWORD_SIZE, formMap["password"]);

	try {
		std::string ipStr = formMap.at("ip");
		if (ipStr.empty()) {
			ESP_LOGD(TAG, "No IP address using default 0.0.0.0");
			connectionInfo.ipInfo.ip.addr = 0;
		} else {
			inet_pton(AF_INET, ipStr.c_str(), &connectionInfo.ipInfo.ip);
		}
	} catch(std::out_of_range& e) {
		ESP_LOGD(TAG, "No IP address using default 0.0.0.0");
		connectionInfo.ipInfo.ip.addr = 0;
	}

	try {
	    std::string gwStr = formMap.at("gw");
	    if (gwStr.empty()) {
		ESP_LOGD(TAG, "No GW address using default 0.0.0.0");
		connectionInfo.ipInfo.gw.addr = 0;
	    } else {
		inet_pton(AF_INET, gwStr.c_str(), &connectionInfo.ipInfo.gw);
	    }
	} catch(std::out_of_range& e) {
	    ESP_LOGD(TAG, "No GW address using default 0.0.0.0");
	    connectionInfo.ipInfo.gw.addr = 0;
	}

	try {
	    std::string netmaskStr = formMap.at("netmask");
	    if (netmaskStr.empty()) {
		ESP_LOGD(TAG, "No Netmask address using default 0.0.0.0");
		connectionInfo.ipInfo.netmask.addr = 0;
	    } else {
		inet_pton(AF_INET, netmaskStr.c_str(), &connectionInfo.ipInfo.netmask);
	    }
	} catch(std::out_of_range& e) {
	    ESP_LOGD(TAG, "No Netmask address using default 0.0.0.0");
	    connectionInfo.ipInfo.netmask.addr = 0;
	}

	ESP_LOGD(TAG, "ssid: %s, password: %s", connectionInfo.ssid, connectionInfo.password);

	saveConnectionInfo(&connectionInfo);

	pResponse->setStatus(HttpResponse::HTTP_STATUS_OK, "OK");
	pResponse->addHeader(HttpRequest::HTTP_HEADER_CONTENT_TYPE, "text/html");
	//pResponse->sendData(std::string((char*)selectAP_html, selectAP_html_len));
	pResponse->close();
	FreeRTOS::sleep(500);
	System::restart();
	ESP_LOGD(TAG, "<< processForm");
} // processForm

static void handleScan(HttpRequest* pRequest, HttpResponse* pResponse) {
	// send back the apList
	// print all the aps
	pResponse->setStatus(HttpResponse::HTTP_STATUS_OK, "OK");
	pResponse->addHeader(HttpRequest::HTTP_HEADER_CONTENT_TYPE, "text/html");
	pResponse->sendData(std::string((char*)selectAP_html, selectAP_html_len));
	pResponse->close();

	std::vector<WiFiAPRecord>::iterator it;
	for (it = apList.begin(); it != apList.end(); it++)
	{
		ESP_LOGI(TAG, "%s, %d rssi", it->getSSID().c_str() , it->getRSSI());
	}

} // sendForm


class BootWifiEventHandler: public WiFiEventHandler {
public:
	BootWifiEventHandler(BootWiFi *pBootWiFi) {
		m_pBootWiFi = pBootWiFi;
	}

	/**
	 * When the access point logic has started and we are able to receive incoming browser
	 * requests, start the internal HTTP Server.
	 */
	esp_err_t apStart() {
		ESP_LOGD("BootWifiEventHandler", ">> apStart");
		if (m_pBootWiFi->m_httpServerStarted == false) {
			m_pBootWiFi->m_httpServerStarted = true;
			m_pBootWiFi->m_httpServer.addPathHandler("GET",  "/",             sendForm);
			m_pBootWiFi->m_httpServer.addPathHandler("POST", "/ssidSelected", processForm);
			m_pBootWiFi->m_httpServer.addPathHandler("GET", "/scan", handleScan);
			m_pBootWiFi->m_httpServer.start(80);
		}
		ESP_LOGD("BootWifiEventHandler", "<< apStart");
		return ESP_OK;
	} // apStaConnected

	/**
	 * If we fail to connect as a station, then become an access point and then
	 * become a web server.
	 */
	esp_err_t staDisconnected() {
		ESP_LOGD("BootWifiEventHandler", ">> staDisconnected");
		m_pBootWiFi->m_wifi.startAP("Duktape", "Duktape");
		ESP_LOGD("BootWifiEventHandler", "<< staDisconnected");
		return ESP_OK;
	} // staDisconnected


	esp_err_t staGotIp(system_event_sta_got_ip_t event_sta_got_ip) {
		ESP_LOGD("BootWifiEventHandler", ">> staGotIP");
		m_pBootWiFi->m_apConnectionStatus = ESP_OK;  // Set the status to ESP_OK
		m_pBootWiFi->m_completeSemaphore.give();   // If we got an IP address, then we can end the boot process.
	        printf("IP = %s", m_pBootWiFi->m_wifi.getStaIp().c_str());
		return ESP_OK;
	} // staGotIp

private:
	BootWiFi *m_pBootWiFi;
};


/**
 * Boot WiFi
 *
 * @brief Get connected to WiFi
 * 
 * @detailed If SSID & Password were previously saved, connect to the AP.  Otherwise become an AP and start an HTTP server so that the user can set SSID & Password - then save it.
 *
 */
void BootWiFi::bootWiFi2() {
	ESP_LOGD(TAG, ">> bootWiFi2");

	// Check for a GPIO override which occurs when a physical Pin is high
	// during the test.  This can force the ability to check for new configuration
	// even if the existing configured access point is available.
	m_wifi.setWifiEventHandler(new BootWifiEventHandler(this));
	if (checkOverrideGpio() 
#if defined(RESTART_COUNTER)
	    || (restart_count_get() >= MAX_RESTART_TO_CONFIG)
#endif
	    )
	{
		ESP_LOGD(TAG, "- restart count exceeded starting AP mode");
		m_wifi.startAP(m_ssid, m_password);
		// do a scan to see what APs are around
		apList = m_wifi.scan();
	} else {
		// There was no restart count override, proceed as normal.  This means we retrieve
		// our stored access point information of the access point we should connect
		// against.  If that information doesn't exist, then again we become an
		// access point ourselves in order to allow a client to connect and bring
		// up a browser.
		connection_info_t connectionInfo;
		int rc = getConnectionInfo(&connectionInfo);
		if (rc == 0) {
			// We have received connection information, let us now become a station
			// and attempt to connect to the access point.
			ESP_LOGD(TAG, "- Connecting to access point \"%s\" ...", connectionInfo.ssid);
			assert(strlen(connectionInfo.ssid) > 0);

		 	m_wifi.setIPInfo(
		 		(uint32_t) connectionInfo.ipInfo.ip.addr,
				(uint32_t) connectionInfo.ipInfo.gw.addr,
				(uint32_t) connectionInfo.ipInfo.netmask.addr
			);
		  
		 	m_apConnectionStatus = m_wifi.connectAP(connectionInfo.ssid, connectionInfo.password);   // Try to connect to the access point.
		 	m_completeSemaphore.give();                                                              // end the boot process so we don't hang...

		} else {
			// We do NOT have connection information.  Let us now become an access
			// point that serves up a web server and allow a browser user to specify
			// the details that will be eventually used to allow us to connect
			// as a station.
			m_wifi.startAP(m_ssid, m_password);
			apList = m_wifi.scan();
		} // We do NOT have connection info
	}
	ESP_LOGD(TAG, "<< bootWiFi2");
} // bootWiFi2


/**
 * @brief Set the userid/password pair that will be used for the ESP32 access point.
 * @param [in] ssid The network id of the ESP32 when it becomes an access point.
 * @param [in] password The password for the ESP32 when it becomes an access point.
 */
void BootWiFi::setAccessPointCredentials(std::string ssid, std::string password) {
	m_ssid     = ssid;
	m_password = password;
} // setAccessPointCredentials



/**
 * @brief Main entry point into booting WiFi - see BootWiFi2 for more detail.
 *
 * The event handler will be called back with the outcome of the connection.
 *
 * @returns ESP_OK if successfully connected to an access point.  Otherwise returns wifi_err_reason_t - to print use GeneralUtils::wifiErrorToString
 */
uint8_t BootWiFi::boot() {
	ESP_LOGD(TAG, ">> boot");
	ESP_LOGD(TAG, " +----------+");
	ESP_LOGD(TAG, " | BootWiFi |");
	ESP_LOGD(TAG, " +----------+");
	ESP_LOGD(TAG, " Access point credentials: %s/%s", m_ssid.c_str(), m_password.c_str());
 	m_completeSemaphore.take("boot");     // Take the semaphore which will be unlocked when we complete booting.
	bootWiFi2();
	m_completeSemaphore.wait("boot");     // Wait for the semaphore that indicated we have completed booting.
	m_wifi.setWifiEventHandler(nullptr);  // Remove the WiFi boot handler when we have completed booting.
	ESP_LOGD(TAG, "<< boot");
  return m_apConnectionStatus;
} // boot

BootWiFi::BootWiFi() {
	m_httpServerStarted = false;
	m_apConnectionStatus = UINT8_MAX;
	uint8_t myApMac[6];
	esp_read_mac(myApMac, ESP_MAC_WIFI_SOFTAP);
	char ssid[32] = "";
	sprintf(ssid, "esp32-%02x%02x", myApMac[4], myApMac[5]);
	ESP_LOGI(TAG, "SoftAp = [%s]\n", ssid);
	setAccessPointCredentials(ssid, "password");   // Default access point credentials
}

BootWiFi::BootWiFi(char *ssidBase) {
	m_httpServerStarted = false;
	m_apConnectionStatus = UINT8_MAX;
	uint8_t myApMac[6];
	esp_read_mac(myApMac, ESP_MAC_WIFI_SOFTAP);
	char ssid[32] = "";
	sprintf(ssid, "%s%02x%02x", ssidBase, myApMac[4], myApMac[5]);
	ESP_LOGI(TAG, "SoftAp = [%s]\n", ssid);
	setAccessPointCredentials(ssid, "password");   // Default access point credentials
}

std::string  BootWiFi::getIp(void)
{
    return (m_wifi.getStaIp());
}

#if defined(RESTART_COUNTER)
static void restart_count_erase_timercb(void *timer)
{
    ESP_LOGI(TAG, "Erase restart callback");
    if (!xTimerStop(timer, portMAX_DELAY)) {
        ESP_LOGE(TAG, "xTimerStop timer: %p", timer);
    }

    if (!xTimerDelete(timer, portMAX_DELAY)) {
        ESP_LOGE(TAG, "xTimerDelete timer: %p", timer);
    }

    NVS myNamespace(BOOTWIFI_NAMESPACE);
    myNamespace.set(KEY_RESTART, 0);
    ESP_LOGD(TAG, "Erase restart count");
}

static int restart_count_get()
{
    TimerHandle_t timer    = NULL;
    uint32_t restart_count = 0;

    /**< If the device restarts within the instruction time,
         the restart_count value will be incremented by one */
    NVS myNamespace(BOOTWIFI_NAMESPACE);
    myNamespace.get(KEY_RESTART, restart_count);
    restart_count++;
    myNamespace.set(KEY_RESTART, restart_count);

    /* create a timer that in 3 seconds will clear the restart counter, so if you restart within 3 secs */
    /* the restart counter will increment. So to get it to increment remove power and add power then remove */
    /* power again, if you continue to reboot within 3 seconds each time then the restart counter will continue */
    /* to incrment, the caller can decide when to do something because of this */
    
    timer = xTimerCreate("restart_count_erase", BOOTWIFI_RESTART_TIMEOUT_MS / portTICK_RATE_MS,
                         false, NULL, restart_count_erase_timercb);
    if (!timer)
    {
    	ESP_LOGE(TAG, "xTaskCreate, timer: %p", timer);
    	return 0;
    }
    
    if (xTimerStart(timer, 0) != pdPASS)
    {
    	ESP_LOGE(TAG, "failed to start");
    }

//    vTaskStartScheduler();
    ESP_LOGD(TAG, "restart count %d", restart_count);
    return restart_count;
}
#endif
