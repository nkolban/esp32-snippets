#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/rmt.h>
#include <stdbool.h>
#include "sdkconfig.h"

static char tag[] = "rmt_tests";
static void dumpStatus(rmt_channel_t channel) {
	bool loop_en;
	uint8_t div_cnt;
	uint8_t memNum;
	bool lowPowerMode;
	rmt_mem_owner_t owner;
	uint16_t idleThreshold;
	uint32_t status;
	rmt_source_clk_t srcClk;

	rmt_get_tx_loop_mode(channel, &loop_en);
	rmt_get_clk_div(channel, &div_cnt);
	rmt_get_mem_block_num(channel, &memNum);
	rmt_get_mem_pd(channel, &lowPowerMode);
	rmt_get_memory_owner(channel, &owner);
	rmt_get_rx_idle_thresh(channel, &idleThreshold);
	rmt_get_status(channel, &status);
	rmt_get_source_clk(channel, &srcClk);

	ESP_LOGD(tag, "Status for RMT channel %d", channel);
	ESP_LOGD(tag, "- Loop enabled: %d", loop_en);
	ESP_LOGD(tag, "- Clock divisor: %d", div_cnt);
	ESP_LOGD(tag, "- Number of memory blocks: %d", memNum);
	ESP_LOGD(tag, "- Low power mode: %d", lowPowerMode);
	ESP_LOGD(tag, "- Memory owner: %s", owner==RMT_MEM_OWNER_TX?"TX":"RX");
	ESP_LOGD(tag, "- Idle threshold: %d", idleThreshold);
	ESP_LOGD(tag, "- Status: %d", status);
	ESP_LOGD(tag, "- Source clock: %s", srcClk==RMT_BASECLK_APB?"APB (80MHz)":"1MHz");
}

void runRmtTest() {
	ESP_LOGD(tag, ">> runRmtTest");

	rmt_config_t config;
	config.rmt_mode = RMT_MODE_TX;
	config.channel = RMT_CHANNEL_0;
	config.gpio_num = 21;
	config.mem_block_num = 1;
	config.tx_config.loop_en = 0;
	config.tx_config.carrier_en = 0;
	config.tx_config.idle_output_en = 1;
	config.tx_config.idle_level = 0;
	config.tx_config.carrier_duty_percent = 50;
	config.tx_config.carrier_freq_hz = 10000;
	config.tx_config.carrier_level = 1;
	config.clk_div = 80;

	ESP_ERROR_CHECK(rmt_config(&config));
	ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
	dumpStatus(config.channel);

	rmt_item32_t items[3];
	items[0].duration0 = 10000;
	items[0].level0 = 1;
	items[0].duration1 = 10000;
	items[0].level1 = 0;

	items[1].duration0 = 10000;
	items[1].level0 = 1;
	items[1].duration1 = 5000;
	items[1].level1 = 0;

	items[2].duration0 = 0;
	items[2].level0 = 1;
	items[2].duration1 = 0;
	items[2].level1 = 0;

	while(1) {
	ESP_ERROR_CHECK(rmt_write_items(config.channel, items,
			3, /* Number of items */
			1 /* wait till done */));
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
	ESP_LOGD(tag, "<< runRmtTest");
}
