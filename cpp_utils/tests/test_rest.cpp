/*
 * Test the REST API client.
 */
#include <curl/curl.h>
#include <esp_log.h>
#include <IFTTT.h>
#include <RESTClient.h>
#include <string>
#include <Task.h>
#include <WiFi.h>
#include <WiFiEventHandler.h>

#include "sdkconfig.h"

static char tag[] = "test_rest";

extern "C" {
	void app_main(void);
}


static WiFi *wifi;

class IFTTTTestTask: public Task {
	void run(void *data) {
		IFTTT iftt = IFTTT("dG-QmEUwCUyBBLHm1owPtq");
		iftt.trigger("test", "A", "BCD", "Val3");
		ESP_LOGD(tag, "trigger done!");
	}
};


class CurlTestTask: public Task {
	void run(void *data) {
		ESP_LOGD(tag, "Testing curl ...");
		RESTClient client;

		/*
		client.setURL("https://httpbin.org/get");
		client.get();
		*/

		/**
		 * Test POST
		 */

		RESTTimings *timings = client.getTimings();

		client.setURL("https://httpbin.org/post");
		client.addHeader("Content-Type", "application/json");
		client.post("hello world!");
		ESP_LOGD(tag, "Result: %s", client.getResponse().c_str());
		timings->refresh();
		ESP_LOGD(tag, "timings: %s", timings->toString().c_str());

		client.setURL("https://httpbin.org/post");
		client.addHeader("Content-Type", "application/json");
		client.post("hello world!");
		ESP_LOGD(tag, "Result: %s", client.getResponse().c_str());
		timings->refresh();
		ESP_LOGD(tag, "timings: %s", timings->toString().c_str());


/*
		client.setURL("https://maker.ifttt.com/trigger/test1/with/key/csrOO1b-w1v32fUSsgwvxq");
		client.addHeader("Content-Type", "application/json");
		client.post("{\"value1\":\"1\",\"value2\":\"2\",\"value3\":\"3\"}");
*/

		printf("Tests done\n");
		return;
	}
};

static CurlTestTask *curlTestTask;

class MyWiFiEventHandler: public WiFiEventHandler {

	esp_err_t staGotIp(system_event_sta_got_ip_t event_sta_got_ip) {
		ESP_LOGD(tag, "MyWiFiEventHandler(Class): staGotIp");

		curlTestTask = new CurlTestTask();
		curlTestTask->setStackSize(12000);
		curlTestTask->start();

		/*
		IFTTTTestTask *pTestTask= new IFTTTTestTask();
		pTestTask->setStackSize(12000);
		pTestTask->start();
		*/
		return ESP_OK;
	}
};


void app_main(void) {
	ESP_LOGD(tag, "app_main: libcurl starting");
	MyWiFiEventHandler *eventHandler = new MyWiFiEventHandler();

	wifi = new WiFi();
	wifi->setWifiEventHandler(eventHandler);

	//wifi->setIPInfo("192.168.1.99", "192.168.1.1", "255.255.255.0");
	//wifi->addDNSServer((char *)"8.8.8.8");

	//wifi->addDNSServer("8.8.4.4");

	wifi->connectAP("sweetie", "l16wint!");
}
