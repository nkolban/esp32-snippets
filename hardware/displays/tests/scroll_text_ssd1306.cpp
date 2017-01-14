#include <driver/spi_master.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "../components/Adafruit_SSD1306-Library/Adafruit_SSD1306.h"
#include "Adafruit_GFX.h"
#include "sdkconfig.h"

static char tag[] = "scroll_text";

Adafruit_SSD1306 display = Adafruit_SSD1306(
	13, // MOSI
	14, // SCLK
	27, // DC
	26, // RST
	15); // CS

extern "C" {
	void task_scroll_text_ssd1306(void *ignore);
}

static void print(char *text) {
	while(*text != 0) {
		display.write(*text);
		text++;
	}
}


void task_scroll_text_ssd1306(void *ignore) {
	ESP_LOGD(tag, "start");
  display.begin();
  // init done

  display.setTextWrap(0);
  display.setTextColor(WHITE);
  int x;
  while(1) {
		for (x=0; x>-80; x-=6) {
			display.clearDisplay();
			display.setCursor(x,5);
			print((char *)"192.168.113.254");
			display.display();
			vTaskDelay(150/portTICK_PERIOD_MS);
			//ESP_LOGD(tag, "next: %d", x);
		}
  }
  ESP_LOGD(tag, "end");
  vTaskDelete(NULL);
}
