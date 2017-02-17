#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <math.h>

#include "sdkconfig.h"

#define PIN_SDA 21
#define PIN_CLK 22
#define I2C_ADDRESS 0x1e

static char tag[] = "hmc5883l";

#undef ESP_ERROR_CHECK
#define ESP_ERROR_CHECK(x)   do { esp_err_t rc = (x); if (rc != ESP_OK) { ESP_LOGE("err", "esp_err_t = %d", rc); assert(0 && #x);} } while(0);

void task_hmc5883l(void *ignore) {
	ESP_LOGD(tag, ">> hmc5883l");
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = PIN_SDA;
	conf.scl_io_num = PIN_CLK;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = 100000;
	ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
	ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));

	uint8_t data[6];

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1);
	i2c_master_write_byte(cmd, 0x02, 1); // Mode register
	i2c_master_write_byte(cmd, 0x00, 1); // value 0
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1);
	i2c_master_write_byte(cmd, 0x01, 1); // Mode register
	i2c_master_write_byte(cmd, 0x20, 1); // value 0
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	cmd = i2c_cmd_link_create();
	ESP_ERROR_CHECK(i2c_master_start(cmd));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 10, 1)); // Data registers
	ESP_ERROR_CHECK(i2c_master_stop(cmd));
	ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 100/portTICK_PERIOD_MS));
	i2c_cmd_link_delete(cmd);

	cmd = i2c_cmd_link_create();
	ESP_ERROR_CHECK(i2c_master_start(cmd));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_READ, 1));
	i2c_master_read_byte(cmd, data,   0);
	i2c_master_read_byte(cmd, data+1, 0);
	i2c_master_read_byte(cmd, data+2, 1);
	ESP_ERROR_CHECK(i2c_master_stop(cmd));
	ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_NUM_0, cmd, 100/portTICK_PERIOD_MS));
	i2c_cmd_link_delete(cmd);

	while(1) {
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_WRITE, 1);
		i2c_master_write_byte(cmd, 0x03, 1); // Data registers
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);

		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (I2C_ADDRESS << 1) | I2C_MASTER_READ, 1);
		i2c_master_read_byte(cmd, data,   0);
		i2c_master_read_byte(cmd, data+1, 0);
		i2c_master_read_byte(cmd, data+2, 0);
		i2c_master_read_byte(cmd, data+3, 0);
		i2c_master_read_byte(cmd, data+4, 0);
		i2c_master_read_byte(cmd, data+5, 1);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000/portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);

		short x = data[0] << 8 | data[1];
		short z = data[2] << 8 | data[3];
		short y = data[4] << 8 | data[5];
		int angle = atan2((double)y,(double)x) * (180 / 3.14159265) + 180; // angle in degrees
		ESP_LOGD(tag, "angle: %d, x: %d, y: %d, z: %d", angle, x, y, z);
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}

	vTaskDelete(NULL);
} // task_hmc5883l
