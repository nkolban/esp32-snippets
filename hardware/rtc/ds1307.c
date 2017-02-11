#include <apps/sntp/sntp.h>
#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <lwip/sockets.h>
#include <stdio.h>
#include <time.h>

#include "sdkconfig.h"

#define SDA_PIN 18
#define SCL_PIN 19
#define DS1307_ADDRESS 0x68

static char tag[] = "ds1307";

static uint8_t intToBCD(uint8_t num) {
	return ((num / 10) << 4) | (num%10);
}

static uint8_t bcdToInt(uint8_t bcd) {
	// 0x10
	return ((bcd >> 4) * 10) + (bcd & 0x0f);;
}

static void startSNTP() {
	ip_addr_t addr;
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	inet_pton(AF_INET, "129.6.15.28", &addr);
	sntp_setserver(0, &addr);
	sntp_init();
}


static void initI2C() {
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
 * The value read from the DS1307 is 7 bytes encoded in BCD:
 * 0 - Seconds - 00-59
 * 1 - Minutes - 00-59
 * 2 - Hours   - 00-23
 * 3 - Day     - 01-07
 * 4 - Date    - 01-31
 * 5 - Month   - 01-12
 * 6 - Year    - 00-99
 *
 */
time_t readValue() {
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	ESP_ERROR_CHECK(i2c_master_start(cmd));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (DS1307_ADDRESS << 1) | I2C_MASTER_WRITE, 1 /* expect ack */));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x0, 1));
	ESP_ERROR_CHECK(i2c_master_start(cmd));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (DS1307_ADDRESS << 1) | I2C_MASTER_READ, 1 /* expect ack */));
	uint8_t data[7];
	ESP_ERROR_CHECK(i2c_master_read(cmd, data, 7, 0));
	ESP_ERROR_CHECK(i2c_master_stop(cmd));
	ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS));
	i2c_cmd_link_delete(cmd);

	int i;
	for (i=0; i<7; i++) {
		ESP_LOGD(tag, "%d: 0x%.2x", i, data[i]);
	}

	struct tm tm;
	tm.tm_sec  = bcdToInt(data[0]);
	tm.tm_min  = bcdToInt(data[1]);
	tm.tm_hour = bcdToInt(data[2]);
	tm.tm_mday = bcdToInt(data[4]);
	tm.tm_mon  = bcdToInt(data[5]) - 1; // 0-11 - Note: The month on the DS1307 is 1-12.
	tm.tm_year = bcdToInt(data[6]) + 100; // Years since 1900
	time_t readTime = mktime(&tm);
	return readTime;
}

void writeValue(time_t newTime) {
	ESP_LOGD(tag, ">> writeValue: %ld", newTime);
	struct tm tm;
	gmtime_r(&newTime, &tm);
	char buf[30];
	ESP_LOGD(tag, " - %s", asctime_r(&tm, buf));

	esp_err_t errRc;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	ESP_ERROR_CHECK(i2c_master_start(cmd));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (DS1307_ADDRESS << 1) | I2C_MASTER_WRITE, 1 /* expect ack */));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x0, 1));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, intToBCD(tm.tm_sec), 1));      // seconds
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, intToBCD(tm.tm_min), 1 ));     // minutes
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, intToBCD(tm.tm_hour), 1 ));    // hours
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, intToBCD(tm.tm_wday+1), 1 ));  // week day
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, intToBCD(tm.tm_mday), 1));     // date of month
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, intToBCD(tm.tm_mon+1), 1));    // month
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, intToBCD(tm.tm_year-100), 1)); // year
	ESP_ERROR_CHECK(i2c_master_stop(cmd));
	errRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	if (errRc != 0) {
		ESP_LOGE(tag, "i2c_master_cmd_begin: %d", errRc);
	}
	i2c_cmd_link_delete(cmd);
}

void task_ds1307(void *ignore) {
	int doWrite = 1;
	ESP_LOGD(tag, ">> ds1307");
	initI2C();
	if (doWrite) {
		startSNTP();
		time_t t;
		while(time(&t) < 1000) {
			ESP_LOGD(tag, "Waiting for SNTP ...");
			vTaskDelay(1000/portTICK_PERIOD_MS);
		}
		writeValue(t);
	}

	while(1) {
		time_t t = time(NULL);
		ESP_LOGD(tag, "time: %ld", t);
		t = readValue();
		ESP_LOGD(tag, "Read from DS1307: %ld", t);
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}
