/*
 * The implementation of the ESP32 NRF24 SPI handler.
 */
#include "NRF24_spi.h"
#include <SPI.h>

#include <stdint.h>

static SPI *spi;
void NRF24_SPI::begin() {
	spi = new SPI();
	spi->init(SPI::DEFAULT_MOSI_PIN, SPI::DEFAULT_MISO_PIN, SPI::DEFAULT_CLK_PIN, SPI::PIN_NOT_SET);
} // begin


/**
 * @brief Transfer some data in and out of SPI.
 *
 */
uint8_t NRF24_SPI::transfer(uint8_t data) {
	spi->transfer(&data, 1);
	return data;
} // transfer
