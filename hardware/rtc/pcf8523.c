#include <apps/sntp/sntp.h>
#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <lwip/sockets.h>
#include <stdio.h>
#include <time.h>
#include "errorhandle_func.h"

#include "sdkconfig.h"

#define SDA_PIN 23
#define SCL_PIN 22
#define RTC_ADDRESS 0x68 // most I2C rtcs have their address on 0x68. any doubt check with i2c scanner snippet

static char tag[] = "RTC";

static uint8_t intToBCD(uint8_t num) {
	return ((num / 10) << 4) | (num%10);
}

static uint8_t bcdToInt(uint8_t bcd) {
	// 0x10
	return ((bcd >> 4) * 10) + (bcd & 0x0f);;
}


void initI2C() {
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = SDA_PIN;
	conf.scl_io_num = SCL_PIN;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = 100000;
	ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
	ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
}

/*
 * PCF8523 slightly changed its 7 bytes encoded in BCD:
 * 03h - Seconds 	- 00-59
 * 04h - Minutes 	- 00-59
 * 05h - Hours   	- 00-23
 * 06h - monthday	- 01-31
 * 07h - weekday	- 00-06
 * 08h - Month   	- 01-12
 * 09h - Year    	- 00-99
 *
 */
time_t rtc_readValue() {
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	ESP_ERROR_CHECK(i2c_master_start(cmd));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (RTC_ADDRESS << 1) | I2C_MASTER_WRITE, true /* expect ack */));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x03, 1)); // start address
	ESP_ERROR_CHECK(i2c_master_start(cmd));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (RTC_ADDRESS << 1) | I2C_MASTER_READ, true /* expect ack */));
	uint8_t data[7];
	ESP_ERROR_CHECK(i2c_master_read(cmd, data, 7, false));
	ESP_ERROR_CHECK(i2c_master_stop(cmd));
	COMMANDCHECKOKERR(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS),"RTC COMMAND");
	i2c_cmd_link_delete(cmd);

	int i;
	for (i=0; i<7; i++) {
		ESP_LOGD(tag, "%d: 0x%.2x", i, data[i]);
	}

	struct tm tm;
	tm.tm_sec  = bcdToInt(data[0]);
	tm.tm_min  = bcdToInt(data[1]);
	tm.tm_hour = bcdToInt(data[2]);
	tm.tm_mday = bcdToInt(data[3]);
	tm.tm_mon  = bcdToInt(data[5]) - 1; // 0-11 - Note: The month on the PCF8523 is 1-12.
	tm.tm_year = bcdToInt(data[6]) + 100; // Years since 1900
	time_t readTime = mktime(&tm);
	return readTime;
}

void rtc_writeValue(time_t newTime) {
	ESP_LOGD(tag, ">> writeValue: %ld", newTime);
	struct tm tm;
	gmtime_r(&newTime, &tm);
	char buf[30];
	ESP_LOGD(tag, " - %s", asctime_r(&tm, buf));

	esp_err_t errRc;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	ESP_ERROR_CHECK(i2c_master_start(cmd));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (RTC_ADDRESS << 1) | I2C_MASTER_WRITE, 1 /* expect ack */));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x03, 1));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, intToBCD(tm.tm_sec), 1));      // seconds
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, intToBCD(tm.tm_min), 1 ));     // minutes
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, intToBCD(tm.tm_hour), 1 ));    // hours
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, intToBCD(tm.tm_mday), 1));     // date of month
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, intToBCD(tm.tm_wday+1), 1 ));  // week day
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, intToBCD(tm.tm_mon+1), 1));    // month
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, intToBCD(tm.tm_year-100), 1)); // year
	ESP_ERROR_CHECK(i2c_master_stop(cmd));
	errRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	if (errRc != 0) {
		ESP_LOGE(tag, "i2c_master_cmd_begin: %d", errRc);
	}
	i2c_cmd_link_delete(cmd);
}


/*
	implement in your time function
	@Kolban created a nice one, here is my contribution.

	esp_err_t sntp_update(){

	static const char *tag = "TIME_SETUP";
	esp_err_t ret;
	char buffer[20];
	EventBits_t bitreturn;
	TickType_t waittime = 10000/portTICK_PERIOD_MS;


	bitreturn = xEventGroupWaitBits(event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, waittime);

	if((bitreturn & BIT0) != 0){

		printf("going online\n");

	}
	else printf("going offline\n");


	// initialize the SNTP service
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	// to create SNTP server variable make a #define statement i.e. "pool.ntp.org"
	sntp_setservername(0, CONFIG_SNTP_SERVER);
	sntp_init();

	initI2C();

	time_t t;
	struct tm timertc;
	t = rtc_readValue();
	localtime_r(&t, &timertc);

	strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", &timertc);
	ESP_LOGI(tag,"Current time in rtc (GTM) is: %s\n\n", buffer);

	// wait for the service to set the time
	time_t now;
	struct tm timeinfo;
	time(&now);
	localtime_r(&now, &timeinfo);
	int counter = 0;

	// try for a minute to get time from network
	while((timeinfo.tm_year < (2018 - 1900)) && (counter < 12))
	{

		ESP_LOGW(tag,"Time outdated, waiting...\n");
		vTaskDelay(5000 / portTICK_PERIOD_MS);
		time(&now);
		localtime_r(&now, &timeinfo);
		counter ++;
	}

	if((timeinfo.tm_year < (2018 - 1900)) && (counter == 12)){
		ESP_LOGE(tag, "TIMEOUT");


		// stick to rtc time
		now = rtc_readValue();

	}
	else{
		// update rtc time
		rtc_writeValue(now);
	}


	// to create timezone variable make a #define statement i.e. "COT+5"
	setenv("TZ",CONFIG_TIMEZONE_TZ, 1);
	tzset();

	// print the actual time in location
	localtime_r(&now, &timeinfo);
	strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", &timeinfo);
	ESP_LOGI(tag,"Current time in your Location: %s\n\n", buffer);

	// check time set in rtc in case it was updated from network
	t = rtc_readValue();
	localtime_r(&t, &timertc);
	strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", &timertc);
	ESP_LOGI(tag,"Current time in rtc (GTM) is: %s\n\n", buffer);

	ret = ESP_OK;

	return ret;
}

 */
