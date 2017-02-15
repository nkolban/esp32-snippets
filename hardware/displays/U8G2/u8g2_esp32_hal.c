#include <stdio.h>
#include <string.h>

#include "sdkconfig.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "u8g2_esp32_hal.h"

static const char *TAG = "u8g2_hal";

static spi_device_handle_t handle; // SPI handle.
static u8g2_esp32_hal_t u8g2_esp32_hal; // HAL state data.

#undef ESP_ERROR_CHECK
#define ESP_ERROR_CHECK(x)   do { esp_err_t rc = (x); if (rc != ESP_OK) { ESP_LOGE("err", "esp_err_t = %d", rc); assert(0 && #x);} } while(0);

/*
 * Initialze the ESP32 HAL.
 */
void u8g2_esp32_hal_init(u8g2_esp32_hal_t u8g2_esp32_hal_param) {
	u8g2_esp32_hal = u8g2_esp32_hal_param;
} // u8g2_esp32_hal_init

/*
 * HAL callback function as prescribed by the U8G2 library.  This callback is invoked
 * to handle callbacks for communications.
 */
uint8_t u8g2_esp32_msg_comms_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
	//ESP_LOGD(tag, "msg_comms_cb: Received a msg: %d: %s", msg, msgToString(msg, arg_int, arg_ptr));
	switch(msg) {
		case U8X8_MSG_BYTE_SET_DC:
			if (u8g2_esp32_hal.dc != U8G2_ESP32_HAL_UNDEFINED) {
				gpio_set_level(u8g2_esp32_hal.dc, arg_int);
			}
			break;

		case U8X8_MSG_BYTE_INIT: {
			if (u8g2_esp32_hal.clk == U8G2_ESP32_HAL_UNDEFINED ||
					u8g2_esp32_hal.mosi == U8G2_ESP32_HAL_UNDEFINED ||
					u8g2_esp32_hal.cs == U8G2_ESP32_HAL_UNDEFINED) {
				break;
			}

		  spi_bus_config_t bus_config;
		  bus_config.sclk_io_num   = u8g2_esp32_hal.clk; // CLK
		  bus_config.mosi_io_num   = u8g2_esp32_hal.mosi; // MOSI
		  bus_config.miso_io_num   = -1; // MISO
		  bus_config.quadwp_io_num = -1; // Not used
		  bus_config.quadhd_io_num = -1; // Not used
		  //ESP_LOGI(tag, "... Initializing bus.");
		  ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &bus_config, 1));

		   break;
		}

		case U8X8_MSG_BYTE_SEND: {
			spi_transaction_t trans_desc;
			trans_desc.address   = 0;
			trans_desc.command   = 0;
			trans_desc.flags     = 0;
			trans_desc.length    = 8 * arg_int; // Number of bits NOT number of bytes.
			trans_desc.rxlength  = 0;
			trans_desc.tx_buffer = arg_ptr;
			trans_desc.rx_buffer = NULL;

			//ESP_LOGI(tag, "... Transmitting %d bytes.", arg_int);
			ESP_ERROR_CHECK(spi_device_transmit(handle, &trans_desc));
			break;
		}
	}
	return 0;
} // u8g2_esp32_msg_comms_cb

/*
 * HAL callback function as prescribed by the U8G2 library.  This callback is invoked
 * to handle callbacks for communications.
 */
uint8_t u8g2_esp32_msg_i2c_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {

	ESP_LOGD(TAG, "msg_i2c_cb: Received a msg: %d %d", msg, arg_int);
	//ESP_LOGD(tag, "msg_i2c_cb: Received a msg: %d: %s", msg, msgToString(msg, arg_int, arg_ptr));

	switch(msg) {
		case U8X8_MSG_BYTE_SET_DC:
			if (u8g2_esp32_hal.dc != U8G2_ESP32_HAL_UNDEFINED) {
				gpio_set_level(u8g2_esp32_hal.dc, arg_int);
			}
			break;

		case U8X8_MSG_BYTE_INIT: {
			if (u8g2_esp32_hal.sda == U8G2_ESP32_HAL_UNDEFINED ||
					u8g2_esp32_hal.scl == U8G2_ESP32_HAL_UNDEFINED) {
				break;
			}

		    i2c_config_t conf;
		    conf.mode = I2C_MODE_MASTER;
			ESP_LOGI(TAG, "sda_io_num %d", u8g2_esp32_hal.sda);
		    conf.sda_io_num = u8g2_esp32_hal.sda;
		    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
			ESP_LOGI(TAG, "scl_io_num %d", u8g2_esp32_hal.scl);
		    conf.scl_io_num = u8g2_esp32_hal.scl;
		    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
			ESP_LOGI(TAG, "clk_speed %d", I2C_MASTER_FREQ_HZ);
		    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
			ESP_LOGI(TAG, "i2c_param_config %d", conf.mode);
		    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
			ESP_LOGI(TAG, "i2c_driver_install %d", I2C_MASTER_NUM);
		    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0));
/*
			i2c_cmd_handle_t cmd = i2c_cmd_link_create(); // dummy write
			ESP_ERROR_CHECK(i2c_master_start(cmd));
			ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00 | I2C_MASTER_WRITE, ACK_CHECK_DIS));
			ESP_ERROR_CHECK(i2c_master_stop(cmd));
			ESP_LOGI(TAG, "i2c_master_cmd_begin %d", I2C_MASTER_NUM);
			ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS));
			i2c_cmd_link_delete(cmd);
*/

			break;
		}

		case U8X8_MSG_BYTE_SEND: {
			uint8_t *data;
			uint8_t cmddata;
			i2c_cmd_handle_t cmd = i2c_cmd_link_create();
			ESP_ERROR_CHECK(i2c_master_start(cmd));
//			ESP_LOGI(TAG, "I2CAddress %02X", u8x8_GetI2CAddress(u8x8)>>1);
			ESP_ERROR_CHECK(i2c_master_write_byte(cmd, u8x8_GetI2CAddress(u8x8) | I2C_MASTER_WRITE, ACK_CHECK_EN));
		    data = (uint8_t *)arg_ptr;
			if (arg_int==1) {
				cmddata=0;
				ESP_ERROR_CHECK(i2c_master_write(cmd, &cmddata, 1, ACK_CHECK_EN));
//				   printf("0x%02X ",zerodata);
			} else {
				cmddata=0x40;
				ESP_ERROR_CHECK(i2c_master_write(cmd, &cmddata, 1, ACK_CHECK_EN));
				//bzero(arg_ptr,arg_int);
				//*data=0x40;
			}
			//ESP_ERROR_CHECK(i2c_master_write(cmd, arg_ptr, arg_int, ACK_CHECK_EN));

		    while( arg_int > 0 ) {
			   ESP_ERROR_CHECK(i2c_master_write_byte(cmd, *data, ACK_CHECK_EN));
//			   printf("0x%02X ",*data);
			   data++;
			   arg_int--;
		    }
//			printf("\n");

			ESP_ERROR_CHECK(i2c_master_stop(cmd));
//			ESP_LOGI(TAG, "i2c_master_cmd_begin %d", I2C_MASTER_NUM);
			ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS));
			i2c_cmd_link_delete(cmd);
			break;
		}
	}
	return 0;
} // u8g2_esp32_msg_i2c_cb

/*
 * HAL callback function as prescribed by the U8G2 library.  This callback is invoked
 * to handle callbacks for GPIO and delay functions.
 */
uint8_t u8g2_esp32_msg_gpio_and_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
	//ESP_LOGD(tag, "msg_gpio_and_delay_cb: Received a msg: %d: %s", msg, msgToString(msg, arg_int, arg_ptr));
	switch(msg) {

	// Initialize the GPIO and DELAY HAL functions.  If the pins for DC and RESET have been
	// specified then we define those pins as GPIO outputs.
		case U8X8_MSG_GPIO_AND_DELAY_INIT: {
			uint64_t bitmask = 0;
			if (u8g2_esp32_hal.dc != U8G2_ESP32_HAL_UNDEFINED) {
				bitmask = bitmask | (1<<u8g2_esp32_hal.dc);
			}
			if (u8g2_esp32_hal.reset != U8G2_ESP32_HAL_UNDEFINED) {
				bitmask = bitmask | (1<<u8g2_esp32_hal.reset);
			}

			gpio_config_t gpioConfig;
			gpioConfig.pin_bit_mask = bitmask;
			gpioConfig.mode         = GPIO_MODE_OUTPUT;
			gpioConfig.pull_up_en   = GPIO_PULLUP_DISABLE;
			gpioConfig.pull_down_en = GPIO_PULLDOWN_ENABLE;
			gpioConfig.intr_type    = GPIO_INTR_DISABLE;
			gpio_config(&gpioConfig);
			break;
		}

	// Set the GPIO reset pin to the value passed in through arg_int.
		case U8X8_MSG_GPIO_RESET:
			if (u8g2_esp32_hal.reset != U8G2_ESP32_HAL_UNDEFINED) {
				gpio_set_level(u8g2_esp32_hal.reset, arg_int);
			}
			break;

	// Delay for the number of milliseconds passed in through arg_int.
		case U8X8_MSG_DELAY_MILLI:
			vTaskDelay(arg_int/portTICK_PERIOD_MS);
			break;
	}
	return 0;
} // u8g2_esp32_msg_gpio_and_delay_cb

/*
 * HAL callback function as prescribed by the U8G2 library.  This callback is invoked
 * to handle callbacks for I²C and delay functions.
 */
uint8_t u8g2_esp32_msg_i2c_and_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {

	ESP_LOGD(TAG, "msg_i2c_and_delay_cb: Received a msg: %d", msg);

	switch(msg) {
	// Initialize the GPIO and DELAY HAL functions.  If the pins for DC and RESET have been
	// specified then we define those pins as GPIO outputs.
		case U8X8_MSG_GPIO_AND_DELAY_INIT: {
			uint64_t bitmask = 0;
			if (u8g2_esp32_hal.dc != U8G2_ESP32_HAL_UNDEFINED) {
				bitmask = bitmask | (1<<u8g2_esp32_hal.dc);
			}
			if (u8g2_esp32_hal.reset != U8G2_ESP32_HAL_UNDEFINED) {
				bitmask = bitmask | (1<<u8g2_esp32_hal.reset);
			}
			if (u8g2_esp32_hal.cs != U8G2_ESP32_HAL_UNDEFINED) {
				bitmask = bitmask | (1<<u8g2_esp32_hal.cs);
			}
			if (u8g2_esp32_hal.sda != U8G2_ESP32_HAL_UNDEFINED) {
				//bitmask = bitmask | (1<<u8g2_esp32_hal.sda);
			}
			if (u8g2_esp32_hal.scl != U8G2_ESP32_HAL_UNDEFINED) {
				//bitmask = bitmask | (1<<u8g2_esp32_hal.scl);
			}
            if (bitmask==0) {
            	break;
            }
			gpio_config_t gpioConfig;
			gpioConfig.pin_bit_mask = bitmask;
			gpioConfig.mode         = GPIO_MODE_OUTPUT;
			gpioConfig.pull_up_en   = GPIO_PULLUP_DISABLE;
			gpioConfig.pull_down_en = GPIO_PULLDOWN_ENABLE;
			gpioConfig.intr_type    = GPIO_INTR_DISABLE;
			gpio_config(&gpioConfig);
			break;
		}

	// Set the GPIO reset pin to the value passed in through arg_int.
		case U8X8_MSG_GPIO_RESET:
			if (u8g2_esp32_hal.reset != U8G2_ESP32_HAL_UNDEFINED) {
				gpio_set_level(u8g2_esp32_hal.reset, arg_int);
			}
			break;
	// Set the GPIO client select pin to the value passed in through arg_int.
		case U8X8_MSG_GPIO_CS:
			if (u8g2_esp32_hal.cs != U8G2_ESP32_HAL_UNDEFINED) {
				gpio_set_level(u8g2_esp32_hal.cs, arg_int);
			}
			break;
	// Set the Software I²C pin to the value passed in through arg_int.
		case U8X8_MSG_GPIO_I2C_CLOCK:
			if (u8g2_esp32_hal.scl != U8G2_ESP32_HAL_UNDEFINED) {
				gpio_set_level(u8g2_esp32_hal.scl, arg_int);
//				printf("%c",(arg_int==1?'C':'c'));
			}
			break;
	// Set the Software I²C pin to the value passed in through arg_int.
		case U8X8_MSG_GPIO_I2C_DATA:
			if (u8g2_esp32_hal.sda != U8G2_ESP32_HAL_UNDEFINED) {
				gpio_set_level(u8g2_esp32_hal.sda, arg_int);
//				printf("%c",(arg_int==1?'D':'d'));
			}
			break;

	// Delay for the number of milliseconds passed in through arg_int.
		case U8X8_MSG_DELAY_MILLI:
			vTaskDelay(arg_int/portTICK_PERIOD_MS);
			break;
	}
	return 0;
} // u8g2_esp32_msg_gpio_and_delay_cb
