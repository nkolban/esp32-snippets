#include <esp_log.h>
#include <string>
#include <WiFi.h>
#include <WiFiEventHandler.h>

#include "sdkconfig.h"

static char tag[] = "my tag";
extern "C" {
	void app_main(void);
}

class MyWiFiEventHandler: public WiFiEventHandler {

	esp_err_t staGotIp(system_event_sta_got_ip_t event_sta_got_ip) {
		ESP_LOGD(tag, "MyWiFiEventHandler(Class): staGotIp");
		// Do something
		return ESP_OK;
	}
};

void app_main(void)
{
	WiFi wifi;
	MyWiFiEventHandler *eventHandler = new MyWiFiEventHandler();

	wifi.setWifiEventHandler(eventHandler);
	wifi.setIPInfo("192.168.1.99", "192.168.1.1", "255.255.255.0");
	wifi.connectAP("myssid", "mypassword");
}
