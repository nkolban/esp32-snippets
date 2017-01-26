#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/rmt.h>
#include "sdkconfig.h"

// Clock divisor (base clock is 80MHz)
#define CLK_DIV 100

// Number of clock ticks that represent 10us.  10 us = 1/100th msec.
#define TICK_10_US (80000000 / CLK_DIV / 100000)

static char tag[] = "rmt_receiver";
static RingbufHandle_t ringBuf;

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

bool isInRange(rmt_item32_t item, int lowDuration, int highDuration, int tolerance) {
	uint32_t lowValue = item.duration0 * 10 / TICK_10_US;
	uint32_t highValue = item.duration1 * 10 / TICK_10_US;
	/*
	ESP_LOGD(tag, "lowValue=%d, highValue=%d, lowDuration=%d, highDuration=%d",
		lowValue, highValue, lowDuration, highDuration);
	*/
	if (lowValue < (lowDuration - tolerance) || lowValue > (lowDuration + tolerance) ||
			(highValue != 0 &&
			(highValue < (highDuration - tolerance) || highValue > (highDuration + tolerance)))) {
		return false;
	}
	return true;
}

bool NEC_is0(rmt_item32_t item) {
	return isInRange(item, 560, 560, 100);
}

bool NEC_is1(rmt_item32_t item) {
	return isInRange(item, 560, 1690, 100);
}

void decodeNEC(rmt_item32_t *data, int numItems) {
	if (!isInRange(data[0], 9000, 4500, 200)) {
		ESP_LOGD(tag, "Not an NEC");
		return;
	}
	int i;
	uint8_t address = 0, notAddress = 0, command = 0, notCommand = 0;
	int accumCounter = 0;
	uint8_t accumValue = 0;
	for (i=1; i<numItems; i++) {
		if (NEC_is0(data[i])) {
			ESP_LOGD(tag, "%d: 0", i);
			accumValue = accumValue >> 1;
		} else if (NEC_is1(data[i])) {
			ESP_LOGD(tag, "%d: 1", i);
			accumValue = (accumValue >> 1) | 0x80;
		} else {
			ESP_LOGD(tag, "Unknown");
		}
		if (accumCounter == 7) {
			accumCounter = 0;
			ESP_LOGD(tag, "Byte: 0x%.2x", accumValue);
			if (i==8) {
				address = accumValue;
			} else if (i==16) {
				notAddress = accumValue;
			} else if (i==24) {
				command = accumValue;
			} else if (i==32) {
				notCommand = accumValue;
			}
			accumValue = 0;
		} else {
			accumCounter++;
		}
	}
	ESP_LOGD(tag, "Address: 0x%.2x, NotAddress: 0x%.2x", address, notAddress ^ 0xff);
	if (address != (notAddress ^ 0xff) || command != (notCommand ^ 0xff)) {
		ESP_LOGD(tag, "Data mis match");
		return;
	}
	ESP_LOGD(tag, "Address: 0x%.2x, Command: 0x%.2x", address, command);
}


static void task_watchRingbuf(void *ignore) {
	size_t itemSize;
	ESP_LOGD(tag, "Watching ringbuf: %d", TICK_10_US);
	while(1) {
		void *data = xRingbufferReceive(ringBuf, &itemSize, portMAX_DELAY);
		ESP_LOGD(tag, "Got an ringbuf item!  Size=%d", itemSize);
		int numItems = itemSize / sizeof(rmt_item32_t);
		int i;
		rmt_item32_t *p = (rmt_item32_t *)data;
		for (i=0; i<numItems; i++) {
			ESP_LOGD(tag, "[0]: %d-%d us, [1]: %d-%d us",
					p->level0, p->duration0 * 10 / TICK_10_US, p->level1, p->duration1 * 10 / TICK_10_US);
			p++;
		}
		decodeNEC((rmt_item32_t *)data, numItems);
		vRingbufferReturnItem(ringBuf, data);
	}
}

void runRmtTest() {
	ESP_LOGD(tag, ">> runRmtTest");

	rmt_config_t config;
	config.rmt_mode = RMT_MODE_RX;
	config.channel = RMT_CHANNEL_0;
	config.gpio_num = 21;
	config.mem_block_num = 2;
	config.rx_config.filter_en = 1;
	config.rx_config.filter_ticks_thresh = 100; // 80000000/100 -> 800000 / 100 = 8000  = 125us
	config.rx_config.idle_threshold = TICK_10_US * 100 * 20;
	config.clk_div = CLK_DIV;

	ESP_ERROR_CHECK(rmt_config(&config));
	ESP_ERROR_CHECK(rmt_driver_install(config.channel, 5000, 0));
	rmt_get_ringbuf_handler(RMT_CHANNEL_0, &ringBuf);
	dumpStatus(config.channel);
  xTaskCreatePinnedToCore(&task_watchRingbuf, "task_watchRingbuf", 2048, NULL, 5, NULL, 0);
  rmt_rx_start(RMT_CHANNEL_0, 1);
	ESP_LOGD(tag, "<< runRmtTest");
}



void task_rmt_receiver(void *ignore) {
	runRmtTest();
	vTaskDelete(NULL);
}
