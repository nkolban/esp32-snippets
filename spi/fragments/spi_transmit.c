#include <driver/spi_master.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "sdkconfig.h"

static char tag[] = "test_spi";

void test_spi_task(void *ignore) {
	ESP_LOGD(tag, ">> test_spi_task");

	spi_bus_config_t bus_config;
	bus_config.sclk_io_num = 21; // CLK
	bus_config.mosi_io_num = 22; // MOSI
	bus_config.miso_io_num = -1; // MISO
	bus_config.quadwp_io_num = -1; // Not used
	bus_config.quadhd_io_num = -1; // Not used
	ESP_LOGI(tag, "... Initializing bus.");
	ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &bus_config, 1));


	spi_device_handle_t handle;
	spi_device_interface_config_t dev_config;
	dev_config.address_bits = 0;
	dev_config.command_bits = 0;
	dev_config.dummy_bits = 0;
	dev_config.mode = 0;
	dev_config.duty_cycle_pos = 0;
	dev_config.cs_ena_posttrans = 0;
	dev_config.cs_ena_pretrans = 0;
	dev_config.clock_speed_hz = 10000;
	dev_config.spics_io_num = 23;
	dev_config.flags = 0;
	dev_config.queue_size = 1;
	dev_config.pre_cb = NULL;
	dev_config.post_cb = NULL;
	ESP_LOGI(tag, "... Adding device bus.");
	ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &dev_config, &handle));


	char data[3];
	spi_transaction_t trans_desc;
	trans_desc.address = 0;
	trans_desc.command = 0;
	trans_desc.flags = 0;
	trans_desc.length = 3 * 8;
	trans_desc.rxlength = 0;
	trans_desc.tx_buffer = data;
	trans_desc.rx_buffer = data;

	data[0] = 0x12;
	data[1] = 0x34;
	data[2] = 0x56;

	ESP_LOGI(tag, "... Transmitting.");
	ESP_ERROR_CHECK(spi_device_transmit(handle, &trans_desc));

	ESP_LOGI(tag, "... Removing device.");
	ESP_ERROR_CHECK(spi_bus_remove_device(handle));

	ESP_LOGI(tag, "... Freeing bus.");
	ESP_ERROR_CHECK(spi_bus_free(HSPI_HOST));

	ESP_LOGD(tag, "<< test_spi_task");
	vTaskDelete(NULL);
}
