/*
 * u8g2_esp32_hal.h
 *
 *  Created on: Feb 12, 2017
 *      Author: kolban
 */

#ifndef U8G2_ESP32_HAL_H_
#define U8G2_ESP32_HAL_H_
#include <driver/gpio.h>
#include <u8g2.h>

#define U8G2_ESP32_HAL_UNDEFINED (-1)

typedef struct {
	gpio_num_t clk;
	gpio_num_t mosi;
	gpio_num_t cs;
	gpio_num_t reset;
	gpio_num_t dc;
} u8g2_esp32_hal_t ;

#define U8G2_ESP32_HAL_DEFAULT {U8G2_ESP32_HAL_UNDEFINED, U8G2_ESP32_HAL_UNDEFINED, U8G2_ESP32_HAL_UNDEFINED, U8G2_ESP32_HAL_UNDEFINED, U8G2_ESP32_HAL_UNDEFINED }

void u8g2_esp32_hal_init(u8g2_esp32_hal_t u8g2_esp32_hal_param);
uint8_t u8g2_esp32_msg_comms_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8g2_esp32_msg_gpio_and_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
#endif /* U8G2_ESP32_HAL_H_ */
