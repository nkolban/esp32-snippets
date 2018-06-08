/*
 * GPIODriver.cpp
 *
 *  Created on: May 27, 2018
 *      Author: kolban
 */

#include "GPIODriver.h"
#include "DMA_GPIO.h"
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define D0    GPIO_NUM_4
#define D1    GPIO_NUM_5
#define D2    GPIO_NUM_18
#define D3    GPIO_NUM_19
#define D4    GPIO_NUM_13 // 36
#define D5    GPIO_NUM_14 // 39
#define D6    GPIO_NUM_15 // 34
#define D7    GPIO_NUM_16 // 35
#define VSYNC GPIO_NUM_25
#define HREF  GPIO_NUM_23
#define PCLK  GPIO_NUM_22

GPIODriver::GPIODriver() {
}

GPIODriver::~GPIODriver() {
}

void GPIODriver::run() {
	printf(">> GPIODriver::run\n");
	// Setup the GPIO pins as output.
	gpio_pad_select_gpio(D0);
	gpio_pad_select_gpio(D1);
	gpio_pad_select_gpio(D2);
	gpio_pad_select_gpio(D3);
	gpio_pad_select_gpio(D4);
	gpio_pad_select_gpio(D5);
	gpio_pad_select_gpio(D6);
	gpio_pad_select_gpio(D7);
	gpio_pad_select_gpio(VSYNC);
	gpio_pad_select_gpio(HREF);
	gpio_pad_select_gpio(PCLK);

	gpio_set_direction(D0,    GPIO_MODE_OUTPUT);
	gpio_set_direction(D1,    GPIO_MODE_OUTPUT);
	gpio_set_direction(D2,    GPIO_MODE_OUTPUT);
	gpio_set_direction(D3,    GPIO_MODE_OUTPUT);
	gpio_set_direction(D4,    GPIO_MODE_OUTPUT);
	gpio_set_direction(D5,    GPIO_MODE_OUTPUT);
	gpio_set_direction(D6,    GPIO_MODE_OUTPUT);
	gpio_set_direction(D7,    GPIO_MODE_OUTPUT);
	gpio_set_direction(VSYNC, GPIO_MODE_OUTPUT);
	gpio_set_direction(HREF,  GPIO_MODE_OUTPUT);
	gpio_set_direction(PCLK,  GPIO_MODE_OUTPUT);

	gpio_set_level(VSYNC, 1);
	gpio_set_level(HREF,  1);
	// Initialize the counter.
	uint8_t counter = 0;
	// Loop

	gpio_set_level(PCLK, 0);
	while(1) {

		//    Write the counter value to the GPIOs
		gpio_set_level(D0, (counter & 0b00000001) != 0);
		gpio_set_level(D1, (counter & 0b00000010) != 0);
		gpio_set_level(D2, (counter & 0b00000100) != 0);
		gpio_set_level(D3, (counter & 0b00001000) != 0);
		gpio_set_level(D4, (counter & 0b00010000) != 0);
		gpio_set_level(D5, (counter & 0b00100000) != 0);
		gpio_set_level(D6, (counter & 0b01000000) != 0);
		gpio_set_level(D7, (counter & 0b10000000) != 0);


		//    Pulse the clock
		gpio_set_level(PCLK, 1);
		vTaskDelay(10/portTICK_PERIOD_MS);
		gpio_set_level(PCLK, 0);

		//    Delay
		vTaskDelay(990/portTICK_PERIOD_MS);
		counter++;
	} // End while 1
} // GPIODriver#run
