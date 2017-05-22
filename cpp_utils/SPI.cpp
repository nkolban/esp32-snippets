/*
 * SPI.cpp
 *
 *  Created on: Mar 3, 2017
 *      Author: kolban
 */

#include "SPI.h"
#include <driver/spi_master.h>
#include <esp_log.h>
#include "sdkconfig.h"

//#define DEBUG 1

static char tag[] = "SPI";
/**
 * @brief Construct an instance of the class.
 *
 * @return N/A.
 */
SPI::SPI() {
	handle = nullptr;
}

/**
 * @brief Class instance destructor.
 */
SPI::~SPI() {
  ESP_LOGI(tag, "... Removing device.");
  ESP_ERROR_CHECK(spi_bus_remove_device(handle));

  ESP_LOGI(tag, "... Freeing bus.");
  ESP_ERROR_CHECK(spi_bus_free(HSPI_HOST));
}

/**
 * @brief Initialize SPI.
 *
 * @param [in] mosiPin Pin to use for MOSI %SPI function.
 * @param [in] misoPin Pin to use for MISO %SPI function.
 * @param [in] clkPin Pin to use for CLK %SPI function.
 * @param [in] csPin Pin to use for CS %SPI function.
 * @return N/A.
 */
void SPI::init(int mosiPin, int misoPin, int clkPin, int csPin) {
	ESP_LOGD(tag, "init: mosi=%d, miso=%d, clk=%d, cs=%d", mosiPin, misoPin, clkPin, csPin);
	spi_bus_config_t bus_config;
	bus_config.sclk_io_num   = clkPin; // CLK
	bus_config.mosi_io_num   = mosiPin; // MOSI
	bus_config.miso_io_num   = misoPin; // MISO
	bus_config.quadwp_io_num = -1; // Not used
	bus_config.quadhd_io_num = -1; // Not used
	ESP_LOGI(tag, "... Initializing bus.");
	ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &bus_config, 1));

	spi_device_interface_config_t dev_config;
	dev_config.address_bits     = 0;
	dev_config.command_bits     = 0;
	dev_config.dummy_bits       = 0;
	dev_config.mode             = 0;
	dev_config.duty_cycle_pos   = 0;
	dev_config.cs_ena_posttrans = 0;
	dev_config.cs_ena_pretrans  = 0;
	dev_config.clock_speed_hz   = 100000;
	dev_config.spics_io_num     = csPin;
	dev_config.flags            = 0;
	dev_config.queue_size       = 1;
	dev_config.pre_cb           = NULL;
	dev_config.post_cb          = NULL;
	ESP_LOGI(tag, "... Adding device bus.");
	ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &dev_config, &handle));
}


/**
 * @brief send and receive data through %SPI.
 *
 * @param [in] data A data buffer used to send and receive.
 * @param [in] dataLen The number of bytes to transmit and receive.
 */
void SPI::transfer(uint8_t *data, size_t dataLen) {
	assert(data != nullptr);
	assert(dataLen > 0);
#ifdef DEBUG
	for (auto i=0; i<dataLen; i++) {
		ESP_LOGD(tag, "> %2d %.2x", i, data[i]);
	}
#endif
	spi_transaction_t trans_desc;
	trans_desc.address   = 0;
	trans_desc.command   = 0;
	trans_desc.flags     = 0;
	trans_desc.length    = dataLen * 8;
	trans_desc.rxlength  = 0;
	trans_desc.tx_buffer = data;
	trans_desc.rx_buffer = data;

	//ESP_LOGI(tag, "... Transferring");
	esp_err_t rc = spi_device_transmit(handle, &trans_desc);
	if (rc != ESP_OK) {
		ESP_LOGE(tag, "transfer:spi_device_transmit: %d", rc);
	}
} // transmit
