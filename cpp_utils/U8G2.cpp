/*
 * U8G2.cpp
 *
 *  Created on: May 6, 2017
 *      Author: kolban
 */
#include "sdkconfig.h"
#ifdef CONFIG_U8G2_PRESENT
#include "U8G2.h"
#include <u8g2.h>
extern "C" {
#include <u8g2_esp32_hal.h>
}
U8G2::U8G2(gpio_num_t sda, gpio_num_t scl, int address) {
	// TODO Auto-generated constructor stub
	u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
	u8g2_esp32_hal.sda = sda;
	u8g2_esp32_hal.scl = scl;
	u8g2_esp32_hal_init(u8g2_esp32_hal);

	u8g2_Setup_ssd1306_128x32_univision_f(
		&m_u8g2,
		U8G2_R0,
		//u8x8_byte_sw_i2c,
		u8g2_esp32_msg_i2c_cb,
		u8g2_esp32_msg_i2c_and_delay_cb);  // init u8g2 structure
	u8x8_SetI2CAddress(&m_u8g2.u8x8,address << 1);
}

U8G2::~U8G2() {
	// TODO Auto-generated destructor stub
}

#endif // CONFIG_U8G2_PRESENT
