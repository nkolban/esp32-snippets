#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <string.h>
#include <u8g2.h>

#include "sdkconfig.h"
#include "u8g2_esp32_hal.h"

//static char tag[] = "u8g2_esp32_hal";

static spi_device_handle_t handle; // SPI handle.
static u8g2_esp32_hal_t u8g2_esp32_hal; // HAL state data.

/*
 * Initialze the ESP32 HAL.
 */
void u8g2_esp32_hal_init(u8g2_esp32_hal_t u8g2_esp32_hal_param) {
	u8g2_esp32_hal = u8g2_esp32_hal_param;
} // u8g2_esp32_hal_init


/*
static char *bytesToString(int length, char *data) {
	static char ret[1024];
	strcpy(ret, "");
	int i;
	char temp[10];
	for (i=0; i<length; i++) {
		sprintf(temp, "%.2x ", data[i]);
		strcat(ret, temp);
	}
	return ret;
}


static char *msgToString(uint8_t msg, uint8_t arg_int, void *arg_ptr) {
	static char buf[1024];
	switch(msg)
	{
	case U8X8_MSG_GPIO_AND_DELAY_INIT:  // called once during init phase of u8g2/u8x8
		return "U8X8_MSG_GPIO_AND_DELAY_INIT";                            // can be used to setup pins

	case U8X8_MSG_DELAY_NANO:           // delay arg_int * 1 nano second
		sprintf(buf, "U8X8_MSG_DELAY_NANO: %d ns", arg_int);
		return buf;

	    case U8X8_MSG_DELAY_100NANO:        // delay arg_int * 100 nano seconds
	      return "U8X8_MSG_DELAY_100NANO";
	    case U8X8_MSG_DELAY_10MICRO:        // delay arg_int * 10 micro seconds
	    	return "U8X8_MSG_DELAY_10MICRO";

	case U8X8_MSG_DELAY_MILLI:          // delay arg_int * 1 milli second
		sprintf(buf, "U8X8_MSG_DELAY_MILLI: %d ms", arg_int);
		return buf;

	    case U8X8_MSG_DELAY_I2C:                // arg_int is the I2C speed in 100KHz, e.g. 4 = 400 KHz
	    	return "U8X8_MSG_DELAY_I2C";
	      break;                            // arg_int=1: delay by 5us, arg_int = 4: delay by 1.25us
	    case U8X8_MSG_GPIO_D0:              // D0 or SPI clock pin: Output level in arg_int
	    //case U8X8_MSG_GPIO_SPI_CLOCK:
	      return "U8X8_MSG_GPIO_D0";
	    case U8X8_MSG_GPIO_D1:              // D1 or SPI data pin: Output level in arg_int
	    //case U8X8_MSG_GPIO_SPI_DATA:
	      return "U8X8_MSG_GPIO_D1";
	    case U8X8_MSG_GPIO_D2:              // D2 pin: Output level in arg_int
	      return "U8X8_MSG_GPIO_D2";
	    case U8X8_MSG_GPIO_D3:              // D3 pin: Output level in arg_int
	      return "U8X8_MSG_GPIO_D3";
	    case U8X8_MSG_GPIO_D4:              // D4 pin: Output level in arg_int
	      return "U8X8_MSG_GPIO_D4";
	    case U8X8_MSG_GPIO_D5:              // D5 pin: Output level in arg_int
	      return "U8X8_MSG_GPIO_D5";
	    case U8X8_MSG_GPIO_D6:              // D6 pin: Output level in arg_int
	      return "U8X8_MSG_GPIO_D6";
	    case U8X8_MSG_GPIO_D7:              // D7 pin: Output level in arg_int
	      return "U8X8_MSG_GPIO_D7";
	    case U8X8_MSG_GPIO_E:               // E/WR pin: Output level in arg_int
	      return "U8X8_MSG_GPIO_E";
	    case U8X8_MSG_GPIO_CS:              // CS (chip select) pin: Output level in arg_int
	      return "U8X8_MSG_GPIO_CS";
	    case U8X8_MSG_GPIO_DC:              // DC (data/cmd, A0, register select) pin: Output level in arg_int
	      return "U8X8_MSG_GPIO_DC";
	 case U8X8_MSG_GPIO_RESET:           // Reset pin: Output level in arg_int
		 sprintf(buf, "U8X8_MSG_GPIO_RESET -> %d", arg_int);
		 return buf;

	    case U8X8_MSG_GPIO_CS1:             // CS1 (chip select) pin: Output level in arg_int
	      return "U8X8_MSG_GPIO_CS1";
	    case U8X8_MSG_GPIO_CS2:             // CS2 (chip select) pin: Output level in arg_int
	      return "U8X8_MSG_GPIO_CS2";
	    case U8X8_MSG_GPIO_I2C_CLOCK:       // arg_int=0: Output low at I2C clock pin
	      return "U8X8_MSG_GPIO_I2C_CLOCK";                            // arg_int=1: Input dir with pullup high for I2C clock pin
	    case U8X8_MSG_GPIO_I2C_DATA:            // arg_int=0: Output low at I2C data pin
	      return "U8X8_MSG_GPIO_I2C_DATA";                            // arg_int=1: Input dir with pullup high for I2C data pin
	    case U8X8_MSG_GPIO_MENU_SELECT:
	      return "U8X8_MSG_GPIO_MENU_SELECT";
	    case U8X8_MSG_GPIO_MENU_NEXT:
	      return "U8X8_MSG_GPIO_MENU_NEXT";
	    case U8X8_MSG_GPIO_MENU_PREV:
	      return "U8X8_MSG_GPIO_MENU_PREV";
	    case U8X8_MSG_GPIO_MENU_HOME:
	    	return "U8X8_MSG_GPIO_MENU_HOME";
	    case U8X8_MSG_BYTE_SEND:
	    	sprintf(buf, "U8X8_MSG_BYTE_SEND: length=%d %s", arg_int, bytesToString(arg_int, arg_ptr));
	    	return buf;
	    case U8X8_MSG_BYTE_INIT:
	    	return "U8X8_MSG_BYTE_INIT";
	    case U8X8_MSG_BYTE_END_TRANSFER:
	    	return "U8X8_MSG_BYTE_END_TRANSFER";
	    case U8X8_MSG_BYTE_START_TRANSFER:
	    	return "U8X8_MSG_BYTE_START_TRANSFER";
	    case U8X8_MSG_BYTE_SET_DC:
	    	sprintf(buf, "U8X8_MSG_BYTE_SET_DC -> %d", arg_int);
	    	return buf;
	    default:
	      return "Unknown";
	  }
}
*/


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


			spi_device_interface_config_t dev_config;
			dev_config.address_bits     = 0;
			dev_config.command_bits     = 0;
			dev_config.dummy_bits       = 0;
			dev_config.mode             = 0;
			dev_config.duty_cycle_pos   = 0;
			dev_config.cs_ena_posttrans = 0;
			dev_config.cs_ena_pretrans  = 0;
			dev_config.clock_speed_hz   = 10000;
			dev_config.spics_io_num     = u8g2_esp32_hal.cs;
			dev_config.flags            = 0;
			dev_config.queue_size       = 200;
			dev_config.pre_cb           = NULL;
			dev_config.post_cb          = NULL;
			//ESP_LOGI(tag, "... Adding device bus.");
			ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &dev_config, &handle));
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
